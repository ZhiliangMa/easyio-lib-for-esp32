#ifndef __UART_CONFIG_H__
#define __UART_CONFIG_H__

#include "driver/uart.h"
#include "string.h"

#define RX_BUF_SIZE (1024)
#define TX_BUF_SIZE (RX_BUF_SIZE)

// Timeout threshold for UART = number of symbols (~10 tics) with unchanged state on receive pin
#define ECHO_READ_TOUT          (3) // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks

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
void uart_init_no_hwfc(uart_port_t uart_num, uint32_t baud_rate, int tx_io_num, int rx_io_num, int queue_size, QueueHandle_t *uart_queue);

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
int uart_sendData(uart_port_t uart_num, const char* data);

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
void uart_rs485_init_with_hwfc(uart_port_t uart_num, uint32_t baud_rate, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num, int queue_size, QueueHandle_t *uart_queue);


#endif
