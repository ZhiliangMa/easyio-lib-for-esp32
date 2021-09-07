
#include "easyio.h"

#define LED 33
#define KEY 0

//ESP-IDF自带的红外解析器配置
#define CONFIG_IR_PROTOCOL_NEC 1
#define CONFIG_IR_PROTOCOL_RC5 0
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

// rmt_ir_tx_task 任务。配置RMT-TX，发射38KHz红外输出，使用ESP-IDF自带的红外编码库，发送单键码和重复码
void rmt_ir_tx_task(void* arg)
{
    uint32_t addr = 0x10;
    uint32_t cmd = 0x20;
    rmt_item32_t *items = NULL;
    uint32_t length = 0;
    ir_builder_t *ir_builder = NULL;

    //配置RMT-TX，发射38KHz红外输出
    rmt_ir_tx_init(RMT1_TX_GPIO, RMT1_TX_CHANNEL);

    //ESP-IDF自带的红外输出编码库
    ir_builder_config_t ir_builder_config = IR_BUILDER_DEFAULT_CONFIG((ir_dev_t)RMT1_TX_CHANNEL);
    ir_builder_config.flags |= IR_TOOLS_FLAGS_PROTO_EXT; // Using extended IR protocols (both NEC and RC5 have extended version)
#if CONFIG_IR_PROTOCOL_NEC
    ir_builder = ir_builder_rmt_new_nec(&ir_builder_config);
#elif CONFIG_IR_PROTOCOL_RC5
    ir_builder = ir_builder_rmt_new_rc5(&ir_builder_config);
#endif

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Send command 0x%x to address 0x%x", cmd, addr);
        // Send new key code
        ESP_ERROR_CHECK(ir_builder->build_frame(ir_builder, addr, cmd));
        ESP_ERROR_CHECK(ir_builder->get_result(ir_builder, &items, &length));
        //To send data according to the waveform items.
        rmt_write_items(RMT1_TX_CHANNEL, items, length, false);

        // Send repeat code
        vTaskDelay(pdMS_TO_TICKS(ir_builder->repeat_period_ms));
        ESP_ERROR_CHECK(ir_builder->build_repeat_frame(ir_builder));
        ESP_ERROR_CHECK(ir_builder->get_result(ir_builder, &items, &length));
        rmt_write_items(RMT1_TX_CHANNEL, items, length, false);
        cmd++;
    }
}

// rmt_ir_rx_task 任务。配置RMT-RX，接收38KHz红外输入，使用ESP-IDF自带的红外解析器进行解析
void rmt_ir_rx_task(void* arg)
{
    rmt_item32_t *frame = NULL; //接收到的帧消息指针
    uint32_t length = 0; //接收到的缓冲区数据长度，单位Byte
    uint32_t numBit = 0; //接收到的数据帧的逻辑位数目，对应着RMT RAM，每个逻辑位用uint32_t的RAM储存

    uint32_t addr = 0; //解析出的地址码
    uint32_t cmd = 0; //解析出的命令码
    bool repeat = false; //重复码标志位

    //配置RMT-RX，接收38KHz红外输入，另外分配环形缓冲区
    rmt_ir_rx_init(RMT0_RX_GPIO, RMT0_RX_CHANNEL, &rmt_rx_ringbuf, 400);

    //ESP-IDF自带的红外解析器配置，只能解析标准NEC/RC5，不能解析空调等非标红外信号
    ir_parser_config_t ir_parser_config = IR_PARSER_DEFAULT_CONFIG((ir_dev_t)RMT0_RX_CHANNEL);
    ir_parser_config.flags |= IR_TOOLS_FLAGS_PROTO_EXT; // Using extended IR protocols (both NEC and RC5 have extended version)
    ir_parser_t *ir_parser = NULL;
#if CONFIG_IR_PROTOCOL_NEC
    ir_parser = ir_parser_rmt_new_nec(&ir_parser_config);
#elif CONFIG_IR_PROTOCOL_RC5
    ir_parser = ir_parser_rmt_new_rc5(&ir_parser_config);
#endif

    while (1) {
        //接收环形缓冲区rmt_rx_ringbuf，返回数据长度length，返回RMT数据指针frame。等待时间为最大
        frame = (rmt_item32_t *) xRingbufferReceive(rmt_rx_ringbuf, &length, (TickType_t)portMAX_DELAY);
        numBit = length / sizeof(rmt_item32_t);
        //调试输出RMT RAM接收到的内容，仅调试用
        rmt_print_ram(frame, numBit);

        //ESP-IDF自带的红外解析，只能解析标准NEC/RC5，不能解析空调等非标红外信号
        if (ir_parser->input(ir_parser, frame, numBit) == ESP_OK) {
            if (ir_parser->get_scan_code(ir_parser, &addr, &cmd, &repeat) == ESP_OK) {
                ESP_LOGI(TAG, "Scan Code %s --- addr: 0x%04x cmd: 0x%04x", repeat ? "(repeat)" : "", addr, cmd);
            }
        }

        //解析数据后，清空ringbuffer。
        vRingbufferReturnItem(rmt_rx_ringbuf, (void *) frame);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 rmt_ir_tx_task 任务，运行任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(rmt_ir_tx_task, "rmt_ir_tx_task", 2048, NULL, 3, NULL);
    // 创建 rmt_ir_rx_task 任务。配置RMT-RX，接收38KHz红外输入，使用ESP-IDF自带的红外解析器进行解析
    xTaskCreate(rmt_ir_rx_task, "rmt_ir_rx_task", 2048, NULL, 3, NULL);
}
