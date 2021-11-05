
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

// 使用片内.jpg文件的首尾地址
extern const uint8_t image_esp32mcu_jpg_start[] asm("_binary_image_esp32mcu_jpg_start");
extern const uint8_t image_esp32mcu_jpg_end[] asm("_binary_image_esp32mcu_jpg_end");

extern const uint8_t image_esp32wifi_jpg_start[] asm("_binary_image_esp32wifi_jpg_start");
extern const uint8_t image_esp32wifi_jpg_end[] asm("_binary_image_esp32wifi_jpg_end");

static const char *TAG = "jpeg";

// jpg_lcd_spi_dma_task 任务。初始化 SPI总线、LCD、jpg解码器，并以`SPI-DMA 双缓冲环形队列`的方式去刷液晶屏，让lcd来回显示两张图片。
void jpg_lcd_spi_dma_task(void* arg)
{
    // 存储着.jpg图片解码后的RGB565像素内容的指针(解码的过程会为它分配内存)
    uint16_t **pimg;

    // 配置SPI3-主机模式，配置DMA通道、DMA字节大小，及 MISO、MOSI、CLK的引脚。
    spi_master_init(VSPI_HOST, LCD_DEF_DMA_CHAN, LCD_DMA_MAX_SIZE, SPI3_DEF_PIN_NUM_MISO, SPI3_DEF_PIN_NUM_MOSI, SPI3_DEF_PIN_NUM_CLK);
    // lcd-驱动IC初始化（注意：普通GPIO最大只能30MHz，而IOMUX默认的SPI引脚，CLK最大可以设置到80MHz）（注意排线不要太长，高速时可能会花屏）
    spi_lcd_init(VSPI_HOST, 80*1000*1000, LCD_SPI3_DEF_PIN_NUM_CS0);

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

    while(1) {
        // 以`SPI-DMA 双缓冲环形队列`的方式去刷液晶屏，解码jpg图片让lcd往复显示两张图片。

        // 解码 image_esp32mcu.jpg，输出RGB565像素格式到内存 pimg
        ret=jpg_decode(image_esp32mcu_jpg_start, &pimg, 0);
        ESP_ERROR_CHECK(ret);
        // 将解码出的RGB565像素格式内容，以SPI-DMA的方式显示到LCD
        lcd_spi_dma_display_img(LCD_SPI, &pimg);
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        // 解码 image_esp32wifi.jpg，输出RGB565像素格式到内存 pimg
        ret=jpg_decode(image_esp32wifi_jpg_start, &pimg, 0);
        ESP_ERROR_CHECK(ret);
        // 将解码出的RGB565像素格式内容，以SPI-DMA的方式显示到LCD
        lcd_spi_dma_display_img(LCD_SPI, &pimg);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 jpg_lcd_spi_dma_task 任务。
    xTaskCreate(jpg_lcd_spi_dma_task, "jpg_lcd_spi_dma_task", 1024*10, NULL, 3, NULL);
}
