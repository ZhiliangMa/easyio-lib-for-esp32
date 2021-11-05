
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

// uart_tx_task 任务。配置uart1-tx，通过串口发送数据
void uart_tx_task(void* arg)
{
    //uart_init_no_hwfc(UART_NUM_2, 115200, GPIO_NUM_15, GPIO_NUM_13, 0, NULL);
    uart_init_no_hwfc(UART_NUM_1, 115200, GPIO_NUM_15, GPIO_NUM_13, 0, NULL);
    while (1) {
        uart_sendData(UART_NUM_1, "uart1, Hello!");
        //uart_sendData(UART_NUM_2, "uart2, Hello!");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

// uart_rx_task 任务。uart1-rx接收数据，使用等待时长的方式去接收数据
void uart_rx_task(void* arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        //const int rxBytes = uart_read_bytes(UART_NUM_2, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
        } else {
            ESP_LOGI(RX_TASK_TAG, "Read 0");
        }
    }
    free(data);
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 uart_tx_task 任务。
    xTaskCreate(uart_tx_task, "uart_tx_task", 2048, NULL, 3, NULL);
    // 创建 uart_rx_task 任务。
    xTaskCreate(uart_rx_task, "uart_rx_task", 2048, NULL, 3, NULL);
}
