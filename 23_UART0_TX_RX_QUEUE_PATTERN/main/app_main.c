
#include "easyio.h"

#define LED 33
#define KEY 0

#define uart_used          UART_NUM_0
#define PATTERN_CHR_NUM    (3)

static const char *TAG = "uart_events";

static QueueHandle_t uart0_queue;

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

// uart_tx_task 任务。配置uart0-txrx，并利用 pattern detect功能实现数据匹配。匹配到 "+++" 字段，会立马将事件发送到队列。
void uart_tx_task(void* arg)
{
    uart_init_no_hwfc(uart_used, 115200, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, 20, &uart0_queue);
    // 设置uart模式检测功能。
    uart_enable_pattern_det_baud_intr(uart_used, '+', PATTERN_CHR_NUM, 9, 0, 0);
    // 重置队列长度以最多记录20个队列位置。？？？20到底是队列的数量？？还是队列单条的长度？？单条发送信息超过20试一下，并且发送进入队列的计数，看会不会有数据遗漏
    // 貌似是单条长度，因为队列数量是在uart_init_no_hwfc中设置的，待定测试。
    uart_pattern_queue_reset(uart_used, 20);

    while (1) {
        //uart_sendData(uart_used, "uart1, Hello!");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

// uart_rx_task 任务。通过串口接收数据，使用队列的方式。实现普通数据接收，和 "+++" 字段检测。
void uart_rx_task(void* arg)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(RX_BUF_SIZE+1);
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            bzero(dtmp, RX_BUF_SIZE);
            ESP_LOGI(TAG, "uart[%d] event:", uart_used);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                    uart_read_bytes(uart_used, dtmp, event.size, portMAX_DELAY);
                    ESP_LOGI(TAG, "[DATA EVT]:");
                    uart_write_bytes(uart_used, (const char*) dtmp, event.size);
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(uart_used);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(uart_used);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    uart_get_buffered_data_len(uart_used, &buffered_size);
                    int pos = uart_pattern_pop_pos(uart_used);
                    ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                    if (pos == -1) {
                        // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                        // record the position. We should set a larger queue size.
                        // As an example, we directly flush the rx buffer here.
                        uart_flush_input(uart_used);
                    } else {
                        uart_read_bytes(uart_used, dtmp, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(uart_used, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                        ESP_LOGI(TAG, "read data: %s", dtmp);
                        ESP_LOGI(TAG, "read pat : %s", pat);
                    }
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
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
