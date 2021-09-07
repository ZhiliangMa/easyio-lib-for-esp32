#include "pulse_cnt.h"

//存储着各PCNT单元的 累计计数值、速度、距离。已根据PCNT单元数量定义过，可直接被外部调用
pcnt_count_t pcnt_count[PCNT_UNIT_MAX];

/* A sample structure to pass events from the PCNT
 * interrupt handler to the main program.
 */
typedef struct {
    int unit;  // the PCNT unit that originated an interrupt
    uint32_t status; // information on the event type that caused the interrupt
} pcnt_evt_t;

//处理 最大/最小值 溢出中断，并已在中断中封装了对累计计数的处理，用户不用担心计数溢出问题。
pcnt_isr_handle_t user_isr_handle = NULL; //user's ISR service handle
pcnt_evt_t evt;
static void IRAM_ATTR pcnt_example_intr_handler(void *arg)
{
    uint32_t intr_status = PCNT.int_st.val;
    int i;
    for (i = 0; i < PCNT_UNIT_MAX; i++) {
        if (intr_status & (BIT(i))) {
            evt.unit = i;
            //Save the PCNT event type that caused an interrupt
            //   to pass it to the main program
            evt.status = PCNT.status_unit[i].val;
            PCNT.int_clr.val = BIT(i);

            //不用队列，以便处理高速计数
            //仅对最大值最小值溢出做处理
            if (evt.status & PCNT_EVT_L_LIM) {
                pcnt_count[i].overflow_num --;
            }
            if (evt.status & PCNT_EVT_H_LIM) {
                pcnt_count[i].overflow_num ++;
            }
            /*if (evt.status & PCNT_EVT_THRES_1) {
                printf("THRES1 EVT\n");
            }
            if (evt.status & PCNT_EVT_THRES_0) {
                printf("THRES0 EVT\n");
            }
            if (evt.status & PCNT_EVT_ZERO) {
                printf("ZERO EVT\n");
            }*/
        }
    }
}

//PCNT用作编码器计数时的统一配置：开启输入滤波前、关闭 两个中间阈值、关闭 过0点 事件监视、使能最大/最小计数值中断、注册中断
static void pcnt_encoder_filter_event_isr_init(pcnt_unit_t unit, uint16_t filter_val)
{
    /* Configure and enable the input filter */
    pcnt_set_filter_value(unit, filter_val);
    pcnt_filter_enable(unit);

    /*//使能事件监视：两个中间阈值 PCNT_THRESH0_VAL、PCNT_THRESH1_VAL
    pcnt_set_event_value(unit, PCNT_EVT_THRES_1, PCNT_THRESH1_VAL);
    pcnt_event_enable(unit, PCNT_EVT_THRES_1);
    pcnt_set_event_value(unit, PCNT_EVT_THRES_0, PCNT_THRESH0_VAL);
    pcnt_event_enable(unit, PCNT_EVT_THRES_0);
    //使能事件监视：过0点
    pcnt_event_enable(unit, PCNT_EVT_ZERO);*/

    //使能事件监视：最大/最小计数值 PCNT_EVT_H_LIM、PCNT_EVT_L_LIM
    pcnt_event_enable(unit, PCNT_EVT_H_LIM);
    pcnt_event_enable(unit, PCNT_EVT_L_LIM);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(unit);
    pcnt_counter_clear(unit);

    /* Register ISR handler and enable interrupts for PCNT unit */
    pcnt_isr_register(pcnt_example_intr_handler, NULL, 0, &user_isr_handle);
    pcnt_intr_enable(unit);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(unit);
}

/**
 * @brief  ABZ正交编码器计数模式，1个旋转周期产生1个计数
 *      - 默认开启 最大/最小值 溢出中断，并已在中断中封装了对累计计数的处理，用户不用担心计数溢出问题
 *      - 如需获取 累计计数、速度、距离，请直接调用cal_pcnt_count_speed_dis(pcnt_unit_t unit, float time, float sub);
 *      - 例：pcnt_encoder_ABZ_1_period_1_cnt_init(PCNT_UNIT_0, 100, ENCODER0_INPUT_A, ENCODER0_INPUT_B);
 *      - 注意：短于信号滤波值的脉冲将被忽略，一般设置为100，1250ns=1.25us，足以允许 1000000/1.25/2=400KHz 的信号输入
 * 
 * @param  unit PCNT单元序号，ESP32有8个PCNT单元，PCNT_UNIT_0~7
 * @param  filter_val 信号滤波值，短于此时间的脉冲将被忽略。以APB_CLK周期为单位，一般为12.5ns。filter_val是一个10Bit的值，范围0~1023。
 * @param  Encoder_IO_A ABZ正交编码器的 A输入通道GPIO
 * @param  Encoder_IO_B ABZ正交编码器的 B输入通道GPIO
 * 
 * @return
 *     - none
 * 
 */
void pcnt_encoder_ABZ_1_period_1_cnt_init(pcnt_unit_t unit, uint16_t filter_val, int Encoder_IO_A, int Encoder_IO_B)
{
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = Encoder_IO_A,
        .ctrl_gpio_num = Encoder_IO_B,
        .channel = PCNT_CHANNEL_0,
        .unit = unit,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = PCNT_H_LIM_VAL,
        .counter_l_lim = PCNT_L_LIM_VAL,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    pcnt_encoder_filter_event_isr_init(unit, filter_val);
}

/**
 * @brief  ABZ正交编码器计数模式，1个旋转周期产生4个计数，位置定位更加精准
 *      - 默认开启 最大/最小值 溢出中断，并已在中断中封装了对累计计数的处理，用户不用担心计数溢出问题
 *      - 如需获取 累计计数、速度、距离，请直接调用cal_pcnt_count_speed_dis(pcnt_unit_t unit, float time, float sub);
 *      - 例：pcnt_encoder_ABZ_1_period_4_cnt_init(PCNT_UNIT_0, 100, ENCODER0_INPUT_A, ENCODER0_INPUT_B);
 *      - 注意：短于信号滤波值的脉冲将被忽略，一般设置为100，1250ns=1.25us，足以允许 1000000/1.25/2=400KHz 的信号输入
 * 
 * @param  unit PCNT单元序号，ESP32有8个PCNT单元，PCNT_UNIT_0~7
 * @param  filter_val 信号滤波值，短于此时间的脉冲将被忽略。以APB_CLK周期为单位，一般为12.5ns。filter_val是一个10Bit的值，范围0~1023。
 * @param  Encoder_IO_A ABZ正交编码器的 A输入通道GPIO
 * @param  Encoder_IO_B ABZ正交编码器的 B输入通道GPIO
 * 
 * @return
 *     - none
 * 
 */
void pcnt_encoder_ABZ_1_period_4_cnt_init(pcnt_unit_t unit, uint16_t filter_val, int Encoder_IO_A, int Encoder_IO_B)
{
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config;

    pcnt_config.pulse_gpio_num = Encoder_IO_A;
    pcnt_config.ctrl_gpio_num  = Encoder_IO_B;
    pcnt_config.channel        = PCNT_CHANNEL_0;
    pcnt_config.unit           = unit;

    pcnt_config.pos_mode       = PCNT_COUNT_INC;    // 上升沿计数++
    pcnt_config.neg_mode       = PCNT_COUNT_DEC;    // 下降沿计数--
    pcnt_config.lctrl_mode     = PCNT_MODE_REVERSE; // CTRL为低电平：反转计数方向
    pcnt_config.hctrl_mode     = PCNT_MODE_KEEP;    // CTRL为高电平：计数方向不变
    pcnt_config.counter_h_lim  = PCNT_H_LIM_VAL;    // 最大计数值
    pcnt_config.counter_l_lim  = PCNT_L_LIM_VAL;    // 最小计数值
    pcnt_unit_config(&pcnt_config);

    pcnt_config.pulse_gpio_num = Encoder_IO_B;
    pcnt_config.ctrl_gpio_num  = Encoder_IO_A;
    pcnt_config.channel        = PCNT_CHANNEL_1;
    pcnt_config.pos_mode       = PCNT_COUNT_DEC;    // 上升沿计数--
    pcnt_config.neg_mode       = PCNT_COUNT_INC;    // 下降沿计数++
    pcnt_unit_config(&pcnt_config);

    pcnt_encoder_filter_event_isr_init(unit, filter_val);
}

/**
 * @brief  Step/Direction 步进脉冲/方向编码器计数模式，1个旋转周期产生2个计数
 *      - 默认开启 最大/最小值 溢出中断，并已在中断中封装了对累计计数的处理，用户不用担心计数溢出问题
 *      - 如需获取 累计计数、速度、距离，请直接调用cal_pcnt_count_speed_dis(pcnt_unit_t unit, float time, float sub);
 *      - 例：pcnt_encoder_Step_Direction_1_period_2_cnt_init(PCNT_UNIT_0, 100, ENCODER0_INPUT_A, ENCODER0_INPUT_B);
 *      - 注意：短于信号滤波值的脉冲将被忽略，一般设置为100，1250ns=1.25us，足以允许 1000000/1.25/2=400KHz 的信号输入
 * 
 * @param  unit PCNT单元序号，ESP32有8个PCNT单元，PCNT_UNIT_0~7
 * @param  filter_val 信号滤波值，短于此时间的脉冲将被忽略。以APB_CLK周期为单位，一般为12.5ns。filter_val是一个10Bit的值，范围0~1023。
 * @param  Encoder_IO_STEP 编码器的 Step步进脉冲 输入通道GPIO
 * @param  Encoder_IO_DIR 编码器的 Direction方向 输入通道GPIO
 * 
 * @return
 *     - none
 * 
 */
void pcnt_encoder_Step_Direction_1_period_2_cnt_init(pcnt_unit_t unit, uint16_t filter_val, int Encoder_IO_STEP, int Encoder_IO_DIR)
{
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config;

    pcnt_config.pulse_gpio_num = Encoder_IO_STEP;
    pcnt_config.ctrl_gpio_num  = Encoder_IO_DIR;
    pcnt_config.channel        = PCNT_CHANNEL_0;
    pcnt_config.unit           = unit;

    pcnt_config.pos_mode       = PCNT_COUNT_INC;    // 上升沿计数++
    pcnt_config.neg_mode       = PCNT_COUNT_INC;    // 下降沿计数++
    pcnt_config.lctrl_mode     = PCNT_MODE_REVERSE; // CTRL为低电平：反转计数方向
    pcnt_config.hctrl_mode     = PCNT_MODE_KEEP;    // CTRL为高电平：计数方向不变
    pcnt_config.counter_h_lim  = PCNT_H_LIM_VAL;    // 最大计数值
    pcnt_config.counter_l_lim  = PCNT_L_LIM_VAL;    // 最小计数值
    pcnt_unit_config(&pcnt_config);

    pcnt_encoder_filter_event_isr_init(unit, filter_val);
}

/**
 * @brief  计算编码器输入的 累计计数、速度、距离
 *      - 默认开启 最大/最小值 溢出中断，并已在中断中封装了对累计计数的处理，用户不用担心计数溢出问题。
 *      - 初始化编码器模式后，直接调用此API即可获得 编码器输入的 累计计数、速度、距离
 *      - 例：cal_pcnt_count_speed_dis(PCNT_UNIT_0, 1, 200); // 时间系数为1秒，距离系数为200。（累计计数200，即行走1mm、1cm、1m，距离单位用户自己定）
 *      - 注意：输入的时间系数，要与该函数的调用周期匹配，才能计算出精准的 speed速度
 *      - 注意：输入的距离系数，要与齿轮减速比、轮胎周长匹配，才能计算出精准的 dis距离
 * 
 * @param  unit PCNT单元序号，ESP32有8个PCNT单元，PCNT_UNIT_0~7
 * @param  time 时间系数，单位s。要与该函数的调用周期匹配，才能计算出精准的 speed速度
 * @param  sub 距离系数，单位用户自己定义。要与齿轮减速比、轮胎周长匹配，才能计算出精准的 dis距离
 * 
 * @return
 *     - none
 * 
 */
void cal_pcnt_count_speed_dis(pcnt_unit_t unit, float time, float sub)
{
    int16_t count = 0;
    //获得PCNT计数值
    pcnt_get_counter_value(unit, &count);
    //计算 累计计数值 = 溢出次数*PCNT_H_LIM_VAL + 当前PCNT的计数值
    pcnt_count[unit].count = pcnt_count[unit].overflow_num * PCNT_H_LIM_VAL + count;
    //计算 速度 = （double）（累计计数值 - 上次计数值） / 距离系数 / 时间系数
    pcnt_count[unit].speed = (double)(pcnt_count[unit].count - pcnt_count[unit].count_last) / sub / time;
    //计算 距离 = （double）（累计计数值） / 距离系数
    pcnt_count[unit].dis = (double)(pcnt_count[unit].count) / sub;
    //记录历史计数值
    pcnt_count[unit].count_last = pcnt_count[unit].count;
}
