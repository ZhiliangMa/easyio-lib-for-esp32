#ifndef __MCPWM_CAPTURE_H__
#define __MCPWM_CAPTURE_H__

#include "easyio_mcpwm_config.h"
#include <stdarg.h>

#define CAP_CHANNEL_NUM     6   //输入捕获的通道数目

//定时器捕获的数据结构体，用来存储6个通道的 获值、跳变沿类型、运算周期频率占空比等数据
typedef struct {
    uint32_t edge_type[CAP_CHANNEL_NUM]; //本次捕获的跳变沿类型，详情查看mcpwm_capture_on_edge_t，下降沿为0，上升沿为1。（每次查询寄存器很繁琐，软件标志位会比这种方式会快很多，但不会因为漏掉一个跳变沿导致后续所有结果错误）
    uint32_t neg_edge_val[CAP_CHANNEL_NUM]; //上次下降沿捕获的值
    uint32_t pos_edge_val[CAP_CHANNEL_NUM]; //上次上升沿捕获的值

    uint64_t period[CAP_CHANNEL_NUM]; //与上次上升沿间隔的时间（来计算周期），单位ns。时长可重复溢出，改为uint64_t
    float freq[CAP_CHANNEL_NUM]; //计算出的脉冲频率，单位Hz。（用上升沿作为参考，如需下降沿需改代码）

    uint64_t high_level_val[CAP_CHANNEL_NUM]; //单个脉宽内高电平的时间，单位ns，用来与周期计算占空比
    float duty_cycle[CAP_CHANNEL_NUM]; //计算出的占空比，单位 %
} capture_unit; //捕获通道的数据
extern capture_unit capture;

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
void mcpwm_capture_duty_cycle_init(int ch_num, ...);


#endif
