
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

#define UART_USED          UART_NUM_1

static const char *TAG = "RS485_ECHO";

// uart_rs485_echo_task 任务。配置RS485端口，配置uartx，硬件流控485，实现RS485消息回传。
void uart_rs485_echo_task(void* arg)
{
    uart_rs485_init_with_hwfc(UART_USED, 115200, GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14, UART_PIN_NO_CHANGE, 0, NULL);

    // Allocate buffers for UART
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);

    while (1) {
        //Read data from UART
        int len = uart_read_bytes(UART_USED, data, RX_BUF_SIZE, 200 / portTICK_RATE_MS);
        if (len > 0) {
            data[len] = 0;
            ESP_LOGI(TAG, "len: %d\tdata: %s", len, data);
            uart_write_bytes(UART_USED, (const char*)&data[0], len);
        }else{
            ESP_LOGE(TAG, "none");
        }
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 uart_rs485_echo_task 任务。
    xTaskCreate(uart_rs485_echo_task, "uart_rs485_echo_task", 2048, NULL, 3, NULL);
}
