
#include "easyio.h"

#define LED 33
#define KEY 0
#define KEY_USR 27

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

static const char *TAG = "main";

// wait_eth_got_ip 任务，等待以太网获得到IP后，会常亮LED
void wait_eth_got_ip(void* arg)
{
    // 阻塞等待 以太网获得到IP的事件
    xEventGroupWaitBits(eth_event_group, GOTIP_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_LOGW(TAG, "Got IP");
    // 注意：使用 eth_event_group 之前，一定要先注册以太网驱动 register_ethernet();

    // 以太网获得IP后，会常亮LED
    vTaskDelete(led_task_Handler);
    led_on(LED);
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    // 注册以太网驱动
    register_ethernet();

    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 wait_eth_got_ip 任务，等待以太网获得IP，用户可在其任务中进行事件处理
    xTaskCreate(wait_eth_got_ip, "wait_eth_got_ip", 4096, NULL, 3, NULL);
}
