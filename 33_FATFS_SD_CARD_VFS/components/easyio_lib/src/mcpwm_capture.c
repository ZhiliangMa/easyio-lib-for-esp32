#include "mcpwm_capture.h"

static int channels[6] = {0}; //解析出的端口列表
static uint32_t cap_basic_val = 0; //时间基数。这个值最后要乘捕获计数值，得到实际时间
static mcpwm_dev_t *MCPWM[2] = {&MCPWM0, &MCPWM1}; //MCPWM寄存器配置的结构体指针
capture_unit capture; //定时器捕获的数据结构体，用来存储6个通道的 获值、跳变沿类型、运算周期频率占空比等数据

//对捕获通道的 捕获值、跳变沿类型、运算周期频率等数据 进行处理的函数
static void capture_deal(mcpwm_unit_t mcpwm_num, mcpwm_capture_signal_t cap_sig, gpio_num_t gpio_num, capture_unit* mcpwm_cap)
{
    //计算位于捕获输入结构体的序列通道
    uint8_t channel = mcpwm_num *3 + cap_sig;
    //获取当前的捕获值
    uint32_t current_cap_val = mcpwm_capture_signal_get_value(mcpwm_num, cap_sig);
    //得到本次捕获的类型：上升沿/下降沿。（没用软件标志位的原因是，不会因为漏掉一个跳变沿而导致后续所有结果错误）
    //mcpwm_cap->edge_type[channel] = mcpwm_capture_signal_get_edge(mcpwm_num, cap_sig);//中断中使用这句必须要进临界区，否则会导致重启。因为比较繁琐，于是干脆用检测GPIO电平的方式
    mcpwm_cap->edge_type[channel] = gpio_get_level(gpio_num);

    //if (mcpwm_cap->edge_type[channel] == MCPWM_POS_EDGE) { //如果是上升沿，来计算周期
    if (mcpwm_cap->edge_type[channel] == 1) { //如果是上升沿，来计算周期
        //求与上次上升沿间隔的时间（周期，单位ns），来计算 周期
        if(current_cap_val > mcpwm_cap->pos_edge_val[channel]) {
            mcpwm_cap->period[channel] = (current_cap_val - mcpwm_cap->pos_edge_val[channel]) * cap_basic_val / 10; //求与上次间隔时间，单位ns
        } else {
            mcpwm_cap->period[channel] = (uint64_t)((uint64_t)current_cap_val + 4294967296 - mcpwm_cap->pos_edge_val[channel]) * cap_basic_val / 10; //求与上次间隔时间，单位ns
        }
        //计算频率，单位 Hz
        mcpwm_cap->freq[channel] = (double)1000000000/mcpwm_cap->period[channel];
        //记录历史值
        mcpwm_cap->pos_edge_val[channel] = current_cap_val;
        //翻转GPIO触发方式，改为下降沿触发
        mcpwm_capture_enable(mcpwm_num, cap_sig, MCPWM_NEG_EDGE, 0);  //capture signal on negative edge
    } else { //如果是下降沿，来计算占空比
        //求与上次上升沿间隔的时间（单位ns），来计算 单个脉宽内高电平的时间
        if(current_cap_val > mcpwm_cap->pos_edge_val[channel]) {
            mcpwm_cap->high_level_val[channel] = (current_cap_val - mcpwm_cap->pos_edge_val[channel]) * cap_basic_val / 10; //求单个脉宽内高电平的时间，单位ns
        } else {
            mcpwm_cap->high_level_val[channel] = (uint64_t)((uint64_t)current_cap_val + 4294967296 - mcpwm_cap->pos_edge_val[channel]) * cap_basic_val / 10; //求与上次间隔时间，单位ns
        }
        //计算占空比，单位 %
        mcpwm_cap->duty_cycle[channel] = (double)mcpwm_cap->high_level_val[channel]/mcpwm_cap->period[channel]*100;
        //记录历史值
        mcpwm_cap->neg_edge_val[channel] = current_cap_val;
        //翻转GPIO触发方式，改为上升沿触发
        mcpwm_capture_enable(mcpwm_num, cap_sig, MCPWM_POS_EDGE, 0);  //capture signal on rising edge
    }
}

//输入捕获中断处理函数，mcpwm0_capture_isr_handler
//队列处理的方式会使 输入频率受限于FreeRTOS运行速率（ESP-IDF默认的运行频率为100Hz）。故直接在中断中处理捕获值，以实现捕获高频脉冲
//分为两个ISR，是为了加快中断处理的速度，不然每次中断要多一倍的判断
static void IRAM_ATTR mcpwm0_capture_isr_handler(void)
{
    //对标志位的处理必须用if，用switch会在并发时产生漏判
    uint32_t mcpwm_intr_status;
    mcpwm_intr_status = MCPWM[MCPWM_UNIT_0]->int_st.val; //Read interrupt status

    if (mcpwm_intr_status & CAP0_INT_EN) { //Check for interrupt on rising edge on CAP0 signal
        capture_deal(MCPWM_UNIT_0, MCPWM_SELECT_CAP0, channels[0], &capture);
    }
    if (mcpwm_intr_status & CAP1_INT_EN) { //Check for interrupt on rising edge on CAP1 signal
        capture_deal(MCPWM_UNIT_0, MCPWM_SELECT_CAP1, channels[1], &capture);
    }
    if (mcpwm_intr_status & CAP2_INT_EN) { //Check for interrupt on rising edge on CAP2 signal
        capture_deal(MCPWM_UNIT_0, MCPWM_SELECT_CAP2, channels[2], &capture);
    }
    MCPWM[MCPWM_UNIT_0]->int_clr.val = mcpwm_intr_status;
}

//输入捕获中断处理函数，mcpwm1_capture_isr_handler
static void IRAM_ATTR mcpwm1_capture_isr_handler(void)
{
    //对标志位的处理必须用if，用switch会在并发时产生漏判
    uint32_t mcpwm_intr_status;
    mcpwm_intr_status = MCPWM[MCPWM_UNIT_1]->int_st.val; //Read interrupt status

    if (mcpwm_intr_status & CAP0_INT_EN) { //Check for interrupt on rising edge on CAP0 signal
        capture_deal(MCPWM_UNIT_1, MCPWM_SELECT_CAP0, channels[3], &capture);
    }
    if (mcpwm_intr_status & CAP1_INT_EN) { //Check for interrupt on rising edge on CAP1 signal
        capture_deal(MCPWM_UNIT_1, MCPWM_SELECT_CAP1, channels[4], &capture);
    }
    if (mcpwm_intr_status & CAP2_INT_EN) { //Check for interrupt on rising edge on CAP2 signal
        capture_deal(MCPWM_UNIT_1, MCPWM_SELECT_CAP2, channels[5], &capture);
    }
    MCPWM[MCPWM_UNIT_1]->int_clr.val = mcpwm_intr_status;
}

/**
 * @brief  MCPWM输入捕获初始化（最多6路输入捕获IO，每路均可单独测量 高电平脉宽时间、周期、频率、占空比）
 *      - 支持函数重载，支持输入不定数目的通道参数，ESP32-MCPWM输入捕获通道数目最大值为6，总参数数目为 2~7。
 *        例：
 *          mcpwm_capture_duty_cycle_init(1, GPIO_CAPTURE0_IN);
 *          mcpwm_capture_duty_cycle_init(3, GPIO_CAPTURE0_IN, GPIO_CAPTURE1_IN, GPIO_CAPTURE2_IN);
 *          mcpwm_capture_duty_cycle_init(6, GPIO_CAPTURE0_IN, GPIO_CAPTURE1_IN, GPIO_CAPTURE2_IN, GPIO_CAPTURE3_IN, GPIO_CAPTURE4_IN, GPIO_CAPTURE5_IN);
 *      - 因为直接在中断中处理捕获值，可以实现高频脉冲的捕获。绕过了队列处理的方式会使 输入频率受限于FreeRTOS运行速率的限制（ESP-IDF默认的运行频率为100Hz）
 *      - 捕获的Timer与PWM的Timer是独立的，而且捕获的时钟源直接来源于APB 时钟，频率通常为80 MHz，所以捕获的精度和分辨率都非常高。分辨率为 1000000000 / 80000000 = 12.5ns。
 *      - 因为没有找到输入捕获定时器的溢出中断API，所以不能测量周期特别长的脉冲，只能测量1个定时器周期以内的脉冲（2^32 / 80000000 = 53.7s，足够绝大多数应用场合了）。
 * 
 * @param  ch_num   总输入捕获通道的数量。最大值为6。
 * @param  (...)    输入捕获列表。支持不定数目参数，数目为 1~6。值为 任意GPIO IN。
 * 
 * @return
 *      - none
 * 
 */
void mcpwm_capture_duty_cycle_init(int ch_num, ...)
{
    //由输入的不定数目参数，解析出端口值
    va_list va;
    va_start(va, ch_num); // init va, pointing to the first argument
    for(int i=0; i<ch_num; i++) {
        channels[i] = va_arg(va,int); // get the next argument, the type is int
    }
    va_end(va);

    //配置 mcpwm输入捕获的GPIO
    if (ch_num<=3) {
        for(int i=0; i<ch_num; i++) {
            mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_0+i, channels[i]);
            gpio_pulldown_en(channels[i]);    //Enable pull down on CAPx   signal
            mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP0+i, MCPWM_POS_EDGE, 0);  //capture signal on rising edge, prescale = 0 i.e. 800,000,000 counts is equal to one second
        }
    } else {
        for(int i=0; i<3; i++) {
            mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_0+i, channels[i]);
            gpio_pulldown_en(channels[i]);    //Enable pull down on CAPx   signal
            mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP0+i, MCPWM_POS_EDGE, 0);  //capture signal on rising edge, prescale = 0 i.e. 800,000,000 counts is equal to one second
        }
        for(int i=0; i<(ch_num-3); i++) {
            mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM_CAP_0+i, channels[i+3]);
            gpio_pulldown_en(channels[i+3]);    //Enable pull down on CAPx   signal
            mcpwm_capture_enable(MCPWM_UNIT_1, MCPWM_SELECT_CAP0+i, MCPWM_POS_EDGE, 0);  //capture signal on rising edge, prescale = 0 i.e. 800,000,000 counts is equal to one second
        }
    }

    //配置 mcpwm输入捕获的中断使能
    switch (ch_num) {
        case 1:
            MCPWM[MCPWM_UNIT_0]->int_ena.val = CAP0_INT_EN;
            break;
        case 2:
            MCPWM[MCPWM_UNIT_0]->int_ena.val = CAP0_INT_EN | CAP1_INT_EN;
            break;
        case 3:
            MCPWM[MCPWM_UNIT_0]->int_ena.val = CAP0_INT_EN | CAP1_INT_EN | CAP2_INT_EN;
            break;
        case 4:
            MCPWM[MCPWM_UNIT_0]->int_ena.val = CAP0_INT_EN | CAP1_INT_EN | CAP2_INT_EN;
            MCPWM[MCPWM_UNIT_1]->int_ena.val = CAP0_INT_EN;
            break;
        case 5:
            MCPWM[MCPWM_UNIT_0]->int_ena.val = CAP0_INT_EN | CAP1_INT_EN | CAP2_INT_EN;
            MCPWM[MCPWM_UNIT_1]->int_ena.val = CAP0_INT_EN | CAP1_INT_EN;
            break;
        case 6:
            MCPWM[MCPWM_UNIT_0]->int_ena.val = CAP0_INT_EN | CAP1_INT_EN | CAP2_INT_EN;
            MCPWM[MCPWM_UNIT_1]->int_ena.val = CAP0_INT_EN | CAP1_INT_EN | CAP2_INT_EN;
            break;
    }

    //注册中断处理函数
    if (ch_num<=3) {
        mcpwm_isr_register(MCPWM_UNIT_0, mcpwm0_capture_isr_handler, NULL, ESP_INTR_FLAG_IRAM, NULL);  //Set ISR Handler
    } else {
        mcpwm_isr_register(MCPWM_UNIT_0, mcpwm0_capture_isr_handler, NULL, ESP_INTR_FLAG_IRAM, NULL);  //Set ISR Handler
        mcpwm_isr_register(MCPWM_UNIT_1, mcpwm1_capture_isr_handler, NULL, ESP_INTR_FLAG_IRAM, NULL);  //Set ISR Handler
    }

    cap_basic_val = 10000000000 / rtc_clk_apb_freq_get(); //这个值最后要乘捕获计数值，得到实际时间
}
