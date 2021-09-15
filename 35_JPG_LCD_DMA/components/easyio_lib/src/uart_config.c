#include "uart_config.h"

/**
 * @brief  配置uartx，无硬件流控（uartx、波特率、输出端口、队列）
 * 		- 如不使用该引脚，请设置为 UART_PIN_NO_CHANGE
 *      - 如不使用队列，请将 queue_size设置为0，*uart_queue设置为 NULL
 *      - 例：uart_init_no_hwfc(UART_NUM_1, 115200, GPIO_NUM_12, GPIO_NUM_13, 0, NULL);
 * 
 * @param  uart_num uart端口号
 * @param  baud_rate 波特率
 * @param  tx_io_num TXD端口号。除仅能做输入 和 6、7、8、9、10、11之外的任意端口。
 * @param  rx_io_num RXD端口号
 * @param  queue_size 队列大小
 * @param  uart_queue 队列指针
 * 
 * @return
 *     - none
 */
void uart_init_no_hwfc(uart_port_t uart_num, uint32_t baud_rate, int tx_io_num, int rx_io_num, int queue_size, QueueHandle_t *uart_queue)
{
    const uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    //uart_driver_install(uart_num, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_driver_install(uart_num, RX_BUF_SIZE * 2, TX_BUF_SIZE * 2, queue_size, uart_queue, 0);
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, tx_io_num, rx_io_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

/**
 * @brief  uart发送字符串
 *      - 例：uart_sendData(UART_NUM_1, "uart1, Hello!");
 * 
 * @param  uart_num uart端口号
 * @param  data 字符串或数组指针
 * 
 * @return
 *     - none
 */
int uart_sendData(uart_port_t uart_num, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(uart_num, data, len);
    return txBytes;
}

/**
 * @brief  配置RS485端口，配置uartx，硬件流控（uartx、波特率、输出端口、队列）
 * 		- 如不使用该引脚，请设置为 UART_PIN_NO_CHANGE
 *      - 如不使用队列，请将 queue_size设置为0，*uart_queue设置为 NULL
 *      - RS485通信，使用硬件流控，只需RTS引脚（Require To Send，发送请求）
 *      - 例：uart_rs485_init_with_hwfc(UART_NUM_1, 115200, GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14, UART_PIN_NO_CHANGE, 0, NULL);
 * 
 * @param  uart_num uart端口号
 * @param  baud_rate 波特率
 * @param  tx_io_num TXD端口号。除仅能做输入 和 6、7、8、9、10、11之外的任意端口。
 * @param  rx_io_num RXD端口号
 * @param  rts_io_num RTS端口号
 * @param  cts_io_num CTS端口号
 * @param  queue_size 队列大小
 * @param  uart_queue 队列指针
 * 
 * @return
 *     - none
 */
void uart_rs485_init_with_hwfc(uart_port_t uart_num, uint32_t baud_rate, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num, int queue_size, QueueHandle_t *uart_queue)
{
    const uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_APB,
    };

    //uart_driver_install(uart_num, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_driver_install(uart_num, RX_BUF_SIZE * 2, TX_BUF_SIZE * 2, queue_size, uart_queue, 0);
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, tx_io_num, rx_io_num, rts_io_num, cts_io_num);

    // Set RS485 half duplex mode
    // 半双工模式下，发送数据时不接收。RS422全双工不需要流控，直接套用 uart_init_no_hwfc
    uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX);
    // Set read timeout of UART TOUT feature
    uart_set_rx_timeout(uart_num, ECHO_READ_TOUT);
}
