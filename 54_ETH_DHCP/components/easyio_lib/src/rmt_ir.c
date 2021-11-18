#include "rmt_ir.h"

RingbufHandle_t rmt_rx_ringbuf = NULL;

/**
 * @brief Default configuration for RX channel
 *
 */
static void RMT_38K_RX_CONFIG(gpio_num_t gpio_num, rmt_channel_t channel_id)
{
	rmt_config_t rmt_rx_config;

	rmt_rx_config.rmt_mode      = RMT_MODE_RX; //RMT RX mode
	rmt_rx_config.channel       = channel_id;  //RMT channel
	rmt_rx_config.gpio_num      = gpio_num;    //GPIO输入管脚
	rmt_rx_config.clk_div       = 80;          //时钟分频80，输入频率为80M/80=1MHz，单个计数时长为1us
	rmt_rx_config.mem_block_num = 1;           //RMT内存块Block数目
	rmt_rx_config.flags         = 0;           //
	rmt_rx_config.rx_config.idle_threshold      = 12000; //当某一电平超过 12000us=12ms时，进入空闲状态
	rmt_rx_config.rx_config.filter_ticks_thresh = 100;   //输入滤波前阈值，100us
	rmt_rx_config.rx_config.filter_en           = true;  //输入滤波，开启

	rmt_config(&rmt_rx_config);
}

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
void rmt_ir_rx_init(gpio_num_t gpio_num, rmt_channel_t channel_id, RingbufHandle_t* ring_buf, size_t rx_buf_size)
{
    //配置RMT-RX，接收38KHz红外输入
    RMT_38K_RX_CONFIG(gpio_num, channel_id);

    //初始化RMT驱动程序，创建环形缓冲区，RMT RX环形缓冲区的大小为rx_buf_size，NEC协议最小配置为150*2，空调的还要大一些，建议实际最小400
    rmt_driver_install(channel_id, rx_buf_size, 0);

    //从RMT单元获取环形缓冲区句柄
    rmt_get_ringbuf_handle(channel_id, ring_buf);
    // RMT启动接收
    rmt_rx_start(channel_id, true);
}

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
void rmt_print_ram(rmt_item32_t *items, uint32_t numItems)
{
	printf("Bit num: %d\n", numItems);
	printf("RAM:\n");
	for (int i=0;i<numItems;i++) {
		//调试输出 每个逻辑位的 电平 : 持续时间(us)    电平 : 持续时间(us)。因为红外接收管的输出形式是开漏，所以需要对电平取反，才是正确的编码输出。
		printf("%d:%d\t%d:%d\n", !(items+i)->level0, (items+i)->duration0, !(items+i)->level1, (items+i)->duration1);
	}
}

/**
 * @brief Default configuration for TX channel
 *
 */
static void RMT_38K_TX_CONFIG(gpio_num_t gpio_num, rmt_channel_t channel_id)
{
	rmt_config_t rmt_tx_config;

	rmt_tx_config.rmt_mode      = RMT_MODE_TX; //RMT TX mode
	rmt_tx_config.channel       = channel_id;  //RMT channel
	rmt_tx_config.gpio_num      = gpio_num;    //GPIO输入管脚
	rmt_tx_config.clk_div       = 80;          //时钟分频80，输入频率为80M/80=1MHz，单个计数时长为1us
	rmt_tx_config.mem_block_num = 1;           //RMT内存块Block数目
	rmt_tx_config.flags         = 0;           //
	rmt_tx_config.tx_config.carrier_freq_hz = 38000;				//载波频率：38KHz
	rmt_tx_config.tx_config.carrier_level = RMT_CARRIER_LEVEL_HIGH; //RMT载波经过调制，可实现高电平输出
	rmt_tx_config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;		//RMT TX空闲电平：低电平
	rmt_tx_config.tx_config.carrier_duty_percent = 33;				//载波百分比，33%（1/3）
	rmt_tx_config.tx_config.carrier_en = true;						//载波使能
	rmt_tx_config.tx_config.loop_en = false;						//不启用循环发送RMT项目
	rmt_tx_config.tx_config.idle_output_en = true;					//RMT空闲电平输出使能

	rmt_config(&rmt_tx_config);
}

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
void rmt_ir_tx_init(gpio_num_t gpio_num, rmt_channel_t channel_id)
{
    //配置RMT-TX，发射38KHz红外输出
    RMT_38K_TX_CONFIG(gpio_num, channel_id);

    //初始化RMT驱动程序，创建环形缓冲区，RMT RX环形缓冲区的大小为rx_buf_size，NEC协议最小配置为150*2，空调的还要大一些，建议实际最小400
    rmt_driver_install(channel_id, 0, 0);
}

/*
//以下部分是打算写个非标空调解码，后来发现空调电视机的编码没有字节概念，长度很随机，要知道具体编码格式。只能等以后有空搞了
static bool nec_bit_check_in_range(rmt_item32_t *item, int32_t high_ticks, int32_t low_ticks, int32_t margin_ticks)
{
	return (item->duration0 < (high_ticks + margin_ticks)) && (item->duration0 > (high_ticks - margin_ticks)) \
	&& (item->duration1 < (low_ticks + margin_ticks)) && (item->duration1 > (low_ticks - margin_ticks));
}

#define IR_MARGIN_US (240)

//不考虑取反，以便可以解析非标码。暂不支持空调，空调电视机的编码没有字节概念，长度很随机，要知道具体编码格式。
bool rmt_ir_rx_decode(rmt_item32_t *items, uint32_t numItems, uint32_t *address, uint32_t *command, bool *repeat)
{
	//记录上一次的 address、command，用以在重复码时返回
	static uint32_t last_address,last_command=0;

	uint32_t add,cmd=0;

	//判断逻辑位数目是否大于1，小于等于1则为失败。
	if (numItems <= 1) {
		return false; //逻辑位数目不正确，放弃此帧。
	}
	//判断是否是重复码，是的话返回重复码，不是的话继续解析
	if ((numItems == 2) && nec_bit_check_in_range(items, NEC_REPEAT_CODE_HIGH_US, NEC_REPEAT_CODE_LOW_US, IR_MARGIN_US) \ 
						nec_bit_check_in_range(items+1, NEC_ENDING_CODE_HIGH_US, 0, IR_MARGIN_US)) {
		//是重复码
		*repeat = true;
		//返回上一次解析出的 address、command
		*address = last_address;
        *command = last_command;
		return true;
	}
	//解析引导码，判断高电平是否在 9000+-240us，低电平是否在 4500+-240us 的时间范围内，是的话捕获引导码成功，开始解析数据位。不是的话放弃此帧。
	if (nec_bit_check_in_range(items, NEC_LEADING_CODE_HIGH_US, NEC_LEADING_CODE_LOW_US, IR_MARGIN_US)) {
		//引导码捕获成功，开始解析 地址码
		for (int i=1;i<(numItems-1);i++) {
			//判断是否为 逻辑0
			if (nec_bit_check_in_range(items+i, NEC_PAYLOAD_ZERO_HIGH_US, NEC_PAYLOAD_ZERO_LOW_US, IR_MARGIN_US)) {
				add =
			}
		}
	} else return false; //引导码不正确，放弃此帧。
}*/
