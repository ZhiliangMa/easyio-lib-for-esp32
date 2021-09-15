
#include "easyio.h"

#define LED 33
#define KEY 0

// 任务句柄，包含创建任务的所有状态，对任务的操作都通过操作任务句柄实现
TaskHandle_t led_task_Handler = NULL;

// led_task 任务，控制LED闪烁
void led_task(void* arg)
{
    // 配置LED为推挽输出，设置初始电平为0
    led_init(LED, 0);
    while(1) {
        // LED状态闪烁
        led_blink(LED);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

static const char *TAG = "jpeg";

// photo_album_task 任务。制作一个用SD卡存储照片的电子相册，它会读取SD卡/pic目录下的所有jpg图片，依次进行解码并显示在`320x240`分辨率的液晶屏上，并循环播放。
void photo_album_task(void* arg)
{
    // 存储着.jpg图片解码后的RGB565像素内容的指针(解码的过程会为它分配内存)
    uint16_t **pimg;

    // 配置SPI3-主机模式，配置DMA通道、DMA字节大小，及 MISO、MOSI、CLK的引脚。
    spi_master_init(VSPI_HOST, LCD_DEF_DMA_CHAN, LCD_DMA_MAX_SIZE, SPI3_DEF_PIN_NUM_MISO, SPI3_DEF_PIN_NUM_MOSI, SPI3_DEF_PIN_NUM_CLK);
    // lcd-驱动IC初始化（注意：普通GPIO最大只能30MHz，而IOMUX默认的SPI引脚，CLK最大可以设置到80MHz）（注意排线不要太长，高速时可能会花屏）
    spi_lcd_init(VSPI_HOST, 1*1000*1000, LCD_SPI3_DEF_PIN_NUM_CS0);

    // 测试画点显示
    // 清屏，用单一底色
    LCD_Clear(WHITE);
    // 显示字符串
    LCD_ShowString(1-1,1-1,YELLOW,BLUE,"Hello! TJpgDec",16,0);

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // 初始化lcd使用`SPI-DMA 双缓冲环形队列`刷屏（申请DMA双缓冲内存）
    lcd_spi_dma_display_init();

    esp_err_t ret;
    // 准备jpg解码器，准备像素格数输出内存（申请jpg解码器、和输出到LCD的RGB像素 需要的内存空间）
    ret=jpg_decode_request_ram(&pimg, lcddev.height+16, lcddev.width+16);
    ESP_ERROR_CHECK(ret);

    // SD卡初始化、FATFS文件系统挂载。总线使用SPI模式，20MHz。
    sdmmc_card_t* card = sd_card_fatfs_spi_init();
    while (!card) { // 验证错误，如果返回值为0，则SD卡初始化及FATFS挂载失败，重试
        ESP_LOGE(TAG, "Failed !! %d Retry!!", false);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        card = sd_card_fatfs_spi_init();
    }

    // 列表显示 /pic目录下所有文件的文件名
    // 如需>8.3的长文件名，需要设置SDK Configuration中的CONFIG_FATFS_LFN_STACK
    DIR *dir;
    struct dirent *ptr;
    dir = opendir(MOUNT_POINT"/pic");
    printf("file list:\n");
    while((ptr = readdir(dir)) != NULL) {
        printf("\t%s\n", ptr->d_name);
    }
    closedir(dir);

    // 用于缓存从SD卡输入的.jpg图片指针，后续根据.jpg的大小为其分配相等的内存
    unsigned char *jpg_img;
    FILE *fr = NULL;
    int size = 0;
    char path[50];

    // 如果解码失败，抛出的异常怎么处理不至于影响到MCU运行
    while(1) {
        // 列表并通过LCD显示 /pic目录下所有文件
        dir = opendir(MOUNT_POINT"/pic");
        while((ptr = readdir(dir)) != NULL) {
            // 打印文件名
            printf("\t%s\n", ptr->d_name);
            // 拼接文件的完整路径
            strcpy(path, MOUNT_POINT"/pic/");
            strcat(path, ptr->d_name);
            printf("\t%s\n", path);
            // 打开文件
            fr = fopen(path, "r");
            //fr = fopen(MOUNT_POINT"/pic/image_esp32mcu.jpg", "r");
            if (fr == NULL) {
                ESP_LOGE(TAG, "Failed to open file for reading");
            }
            // 求得文件的大小
            fseek(fr, 0, SEEK_END);
            size = ftell(fr);
            rewind(fr);
            // 根据文件大小，申请相等容量的内存，用于缓存输入的.jpg图片
            ESP_LOGI(TAG, "malloc: %d", size);
            jpg_img = malloc(size);
            if (jpg_img == NULL) {
                ESP_LOGE(TAG, "Error allocating memory");
                // 如果文件大小超出可用内存，会导致申请内存失败。这时直接跳过此文件，继续去处理下一个。以避免造成重启影响任务运行。
                fclose(fr);
                free(jpg_img);
                continue;
            }
            // 将图片从SD卡内读取RAM中
            fread(jpg_img, size, 1, fr);
            // 关闭文件
            fclose(fr);
            // 解码 image_wave.jpg，输出RGB565像素格式到内存 pimg
            ret=jpg_decode(jpg_img, &pimg, 0);
            // 解码错误的原因，大概率是因为引入了QQ截图的.jpg图片。在此为了任务运行考虑，暂不做处理，直接跳过。
            //ESP_ERROR_CHECK(ret); // 添加了这行，在返回错误时会导致重启
            // 将解码出的RGB565像素格式内容，以SPI-DMA的方式显示到LCD
            lcd_spi_dma_display_img(LCD_SPI, &pimg);
            // 释放申请的内存
            free(jpg_img);

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        closedir(dir);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 photo_album_task 任务。
    xTaskCreate(photo_album_task, "jpg_lcd_spi_dma_effect_task", 1024*10, NULL, 3, NULL);
}
