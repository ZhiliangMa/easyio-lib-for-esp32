
#include "easyio.h"

#define LED 33
#define KEY 0

static const char *TAG = "example";

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

//rmt_ws2812b_task 任务。配置RMT-TX，并安装 ws2812B驱动，实现灯带七彩流光的显示效果
void rmt_ws2812b_task(void* arg)
{
    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;

    // 配置RMT-TX，并安装 ws2812B驱动
    rmt_ws2812b_tx_init(WS2812B_TX_GPIO, WS2812B_TX_CHANNEL, WS2812B_LED_NUMBER);

    while (1) {
        /*// 整条灯带同一颜色。注意：明度要调低，电脑USB未必能输出足够的电流，从而导致带LED灯带不能下载代码
        for (uint16_t i=0;i<WS2812B_LED_NUMBER;i++) {
            // 将HSV颜色空间转换为RGB
            led_strip_hsv2rgb(0, 100, 10, &red, &green, &blue);
            // 将RGB值写入到一个led灯珠的驱动缓存（注意：此函数一次只能编辑一个灯珠的RGB值）
            ESP_ERROR_CHECK(strip->set_pixel(strip, i, red, green, blue));
        }
        // 将整条灯带的RGB缓存值，通过RMT刷新到LED显示。超时时间100ms
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
        vTaskDelay(pdMS_TO_TICKS(2000));*/

        // 五彩流光灯光效果。注意：明度要调低，电脑USB未必能输出足够的电流，从而导致带LED灯带不能下载代码
        for (uint16_t j=0;j<360;j++) {
            for (uint16_t i=0;i<WS2812B_LED_NUMBER;i++) {
                // 将HSV颜色空间转换为RGB
                uint16_t h = 360 * i / WS2812B_LED_NUMBER + j;
                led_strip_hsv2rgb(h, 100, 10, &red, &green, &blue);
                // 将RGB值写入到一个led灯珠的驱动缓存（注意：此函数一次只能编辑一个灯珠的RGB值）
                ESP_ERROR_CHECK(strip->set_pixel(strip, i, red, green, blue));
            }

            // 将整条灯带的RGB缓存值，通过RMT刷新到LED显示。超时时间100ms
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 rmt_ws2812b_task 任务，运行任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(rmt_ws2812b_task, "rmt_ws2812b_task", 2048, NULL, 3, NULL);
}
