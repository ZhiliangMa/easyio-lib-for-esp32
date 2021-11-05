#ifndef __PULSE_CNT_H__
#define __PULSE_CNT_H__

#include "driver/pcnt.h"

//观察点的 最大计数值 和 最小计数值（当PULSE_CNT 计数到最大最小值时，ESP32硬件会自动清零PULSE_CNT，所以这两个值决定了PCNT的计数范围）
#define PCNT_H_LIM_VAL      30000     //PCNT_H_LIM_VAL应设为正数。为了不频繁进入中断，此值应在int16_t的范围内设置的尽可能大
#define PCNT_L_LIM_VAL      PCNT_H_LIM_VAL*-1    //PCNT_L_LIM_VAL应设为负数，与PCNT_H_LIM_VAL*-1一致，方便后期运算

//两个中间阈值，当PULSE_CNT等于这两个值时，会产生相应的 thr_event 信号
#define PCNT_THRESH1_VAL    5
#define PCNT_THRESH0_VAL   -5

//SIG：递增递减计数管脚     CTRL：递增递减方向控制引脚
#define PCNT_INPUT_SIG_IO   4  // Pulse Input GPIO
#define PCNT_INPUT_CTRL_IO  5  // Control GPIO HIGH=count up, LOW=count down

#define ENCODER0_INPUT_A     4  // ABZ正交编码器的 A输入通道GPIO
#define ENCODER0_INPUT_B     5  // ABZ正交编码器的 B输入通道GPIO

#define ENCODER0_INPUT_STEP  4  // 编码器的 Step步进脉冲 输入通道GPIO
#define ENCODER0_INPUT_DIR   5  // 编码器的 Direction方向 输入通道GPIO

//PCNT计算 累计计数、速度、距离 的结构体。已根据PCNT单元数量定义过，可直接被外部调用
typedef struct {
    int64_t count;  // 累计计数值 = 溢出次数 *PCNT_H_LIM_VAL + 当前PCNT的计数值
    int64_t overflow_num;   // 溢出次数
    int64_t count_last;  // 上次计数值
    double speed;    // 速度 = （double）（累计计数值 - 上次计数值） / 距离系数 / 时间系数
    double dis;     // 距离 = （double）（累计计数值） / 距离系数
} pcnt_count_t;
extern pcnt_count_t pcnt_count[PCNT_UNIT_MAX];


/**
 * @brief  ABZ正交编码器计数模式，1个旋转周期产生1个计数
 *      - 默认开启 最大/最小值 溢出中断，并已在中断中封装了对累计计数的处理，用户不用担心计数溢出问题
 *      - 如需获取 累计计数、速度、距离，请直接调用cal_pcnt_count_speed_dis(pcnt_unit_t unit, float time, float sub);
 *      - 例：pcnt_encoder_ABZ_1_period_1_cnt_init(PCNT_UNIT_0, 100, ENCODER_INPUT_A, ENCODER_INPUT_B);
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
void pcnt_encoder_ABZ_1_period_1_cnt_init(pcnt_unit_t unit, uint16_t filter_val, int Encoder_IO_A, int Encoder_IO_B);


/**
 * @brief  ABZ正交编码器计数模式，1个旋转周期产生4个计数，位置定位更加精准
 *      - 默认开启 最大/最小值 溢出中断，并已在中断中封装了对累计计数的处理，用户不用担心计数溢出问题
 *      - 如需获取 累计计数、速度、距离，请直接调用cal_pcnt_count_speed_dis(pcnt_unit_t unit, float time, float sub);
 *      - 例：pcnt_encoder_ABZ_1_period_4_cnt_init(PCNT_UNIT_0, 100, ENCODER_INPUT_A, ENCODER_INPUT_B);
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
void pcnt_encoder_ABZ_1_period_4_cnt_init(pcnt_unit_t unit, uint16_t filter_val, int Encoder_IO_A, int Encoder_IO_B);


/**
 * @brief  Step/Direction 步进脉冲/方向编码器计数模式，1个旋转周期产生2个计数
 *      - 默认开启 最大/最小值 溢出中断，并已在中断中封装了对累计计数的处理，用户不用担心计数溢出问题
 *      - 如需获取 累计计数、速度、距离，请直接调用cal_pcnt_count_speed_dis(pcnt_unit_t unit, float time, float sub);
 *      - 例：pcnt_encoder_Step_Direction_1_period_2_cnt_init(PCNT_UNIT_0, 100, ENCODER_INPUT_A, ENCODER_INPUT_B);
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
void pcnt_encoder_Step_Direction_1_period_2_cnt_init(pcnt_unit_t unit, uint16_t filter_val, int Encoder_IO_STEP, int Encoder_IO_DIR);


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
void cal_pcnt_count_speed_dis(pcnt_unit_t unit, float time, float sub);

#endif
