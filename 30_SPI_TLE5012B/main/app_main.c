
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

// spi_tle5012b_task 任务。初始化 SPI3-TLE5012B，并连续读磁极转子角度。
void spi_tle5012b_task(void* arg)
{
    uint16_t xdata = 0;
    float angle = 0;
    uint16_t speed = 0;

    // 配置SPI3-主机模式，配置DMA通道、DMA字节大小，及 MISO、MOSI、CLK的引脚。
    spi_master_init(SPI3_HOST, LCD_DEF_DMA_CHAN, LCD_DMA_MAX_SIZE, SPI3_DEF_PIN_NUM_MISO, SPI3_DEF_PIN_NUM_MOSI, SPI3_DEF_PIN_NUM_CLK);
    // TLE5012B初始化
    spi_tle5012b_init(SPI3_HOST, 4*1000*1000, TLE5012B_SOFT_CS0); // 4MHz。手册推荐使用的SPI速率为8MHz，实测串联220R电阻速率可以跑到18MHz

    while(1) {
        // 连续读TLE5012B转子角度的原始值
        xdata = tle5012b_read_angle(TLE5012B_SPI, TLE5012B_SOFT_CS0);
        angle = tle5012_to_angle(xdata);
        speed = tle5012b_read_speed(TLE5012B_SPI, TLE5012B_SOFT_CS0);
        printf("data = %d, angle = %.3f°, speed = %d\r\n", xdata, angle, speed);

		vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 spi_tle5012b_task 任务。
    xTaskCreate(spi_tle5012b_task, "spi_tle5012b_task", 2048, NULL, 3, NULL);
}
