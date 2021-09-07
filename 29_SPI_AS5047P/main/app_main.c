
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

// spi_as5047p_task 任务。初始化 SPI3-AS5047P，并连续读磁极转子角度。
void spi_as5047p_task(void* arg)
{
    uint16_t data = 0;
    float angle = 0;

    // 配置SPI3-主机模式，配置DMA通道、DMA字节大小，及 MISO、MOSI、CLK的引脚。
    spi_master_init(SPI3_HOST, LCD_DEF_DMA_CHAN, LCD_DMA_MAX_SIZE, SPI3_DEF_PIN_NUM_MISO, SPI3_DEF_PIN_NUM_MOSI, SPI3_DEF_PIN_NUM_CLK);
    // AS5047P初始化
    spi_as5047p_init(SPI3_HOST, 4*1000*1000, AS5047P_SOFT_CS0); // 4MHz。手册推荐使用的SPI速率为10MHz，实测串联100R电阻速率可以跑到18MHz

    while(1) {
        // 连续读AS5047P磁极转子角度
        data = as5047p_read_angle_continuous(AS5047P_SPI, AS5047P_SOFT_CS0);
        angle = as5047p_to_angle(data);
        printf("data = %d, angle = %.3f°\r\n", data, angle);

		vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 spi_as5047p_task 任务。
    xTaskCreate(spi_as5047p_task, "spi_as5047p_task", 2048, NULL, 3, NULL);
}
