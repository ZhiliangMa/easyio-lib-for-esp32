#ifndef __RMT_IR_H__
#define __RMT_IR_H__

#include "driver/rmt.h"

#define RMT0_RX_CHANNEL    RMT_CHANNEL_0
#define RMT0_RX_GPIO       35
#define RMT1_TX_CHANNEL    RMT_CHANNEL_1
#define RMT1_TX_GPIO       32


extern RingbufHandle_t rmt_rx_ringbuf;

/**
 * @brief  配置RMT-RX，接收38KHz红外输入，另外分配环形缓冲区
 * 		- 暂不支持空调，空调电视机的编码没有字节概念，长度很随机，要知道具体编码格式定制编写。
 *      - rmt_ir.c 提供了一个 RingbufHandle_t rmt_rx_ringbuf; 外部可以直接调用
 *      - 例：rmt_ir_rx_init(RMT0_RX_GPIO, RMT0_RX_CHANNEL, &rmt_rx_ringbuf, 400);
 * 
 * @param  gpio_num RMT-RX红外信号输入管脚号。任意GPIO
 * @param  channel_id 使用的RMT单元号。RMT_CHANNEL_0~7
 * @param  ring_buf 环形缓冲区指针。可以直接使用 rmt_ir.c 提供的 RingbufHandle_t rmt_rx_ringbuf;
 * @param  rx_buf_size 环形缓冲区大小，单位Byte。
 * 
 * @return
 *     - none
 * 
 */
void rmt_ir_rx_init(gpio_num_t gpio_num, rmt_channel_t channel_id, RingbufHandle_t* ring_buf, size_t rx_buf_size);

/**
 * @brief  调试输出RMT RAM接收到的内容，仅调试用
 * 
 * @param  items RMT RAM的指针，rmt_item32_t类型
 * @param  numItems RMT RAM数量
 * 
 * @return
 *     - none
 * 
 */
void rmt_print_ram(rmt_item32_t *items, uint32_t numItems);

/**
 * @brief  配置RMT-TX，发射38KHz红外输出
 * 
 * @param  gpio_num RMT-TX红外信号输出管脚号。除了几个仅能做输入的其他任意GPIO
 * @param  channel_id 使用的RMT单元号。RMT_CHANNEL_0~7
 * 
 * @return
 *     - none
 * 
 */
void rmt_ir_tx_init(gpio_num_t gpio_num, rmt_channel_t channel_id);


#endif
