#ifndef __MCPWM_HALF_BRIDGE_H__
#define __MCPWM_HALF_BRIDGE_H__

#include "mcpwm_motor.h"
#include "easyio_mcpwm_config.h"


/**
 * @brief 可选择的半桥序号，共6个
 */
typedef enum {
    half_bridge0 = 0,   /*!<half_bridge0 selected*/
    half_bridge1,       /*!<half_bridge1 selected*/
    half_bridge2,       /*!<half_bridge2 selected*/
    half_bridge3,       /*!<half_bridge3 selected*/
    half_bridge4,       /*!<half_bridge4 selected*/
    half_bridge5,       /*!<half_bridge5 selected*/
    half_bridge_max,    /*!<Num of Half_Bridge on easyIO*/
} half_bridge_t;

/**
 * @brief 半桥的输出方式：同相 / 反相
 */
typedef enum {
    inphase = 0,        //同相
    inverted,           //反相互补输出
} output_type_t;

/**
 * @brief  半桥PWM驱动初始化及配置(用户层用)
 *      - 每次指定PWM频率 和 2个PWM引脚，供1路半桥驱动使用。（如需多路半桥相位相等，可用"mcpwm_dc_motor_sync"开启相位同步）
 *      - MCPWM的信号可映射到任意GPIO输出管脚。ESP32：0-19, 21-23, 25-27, 32-39。GPIO 34-39 仅用作输入管脚。
 *      - 例：mcpwm_half_bridge_init(half_bridge0, 1000, inverted, GPIO_HALF_BRIDGE0_PWMA_OUT, GPIO_HALF_BRIDGE0_PWMB_OUT);
 *      - 例：mcpwm_half_bridge_init(half_bridge1, 1000, inverted, GPIO_HALF_BRIDGE1_PWMA_OUT, GPIO_HALF_BRIDGE1_PWMB_OUT);
 * 
 * @param  half_bridge_num 半桥驱动序号，half_bridge0~5
 * @param  frequency PWM频率，单位Hz
 * @param  output_type 半桥的PWMA/PWMB输出类型：同相/反相，inphase / inverted
 * @param  PWMxA_gpio_num PWMA管脚Pin
 * @param  PWMxB_gpio_num PWMB管脚Pin
 */
void mcpwm_half_bridge_init(half_bridge_t half_bridge_num, uint16_t frequency, output_type_t output_type, int PWMxA_gpio_num, int PWMxB_gpio_num);

/**
 * @brief  半桥PWM驱动相位同步配置(用户层用)
 *      - 每次指定半桥序号、同步信号、相移千分比、同步触发的外部GPIO引脚，来启动对应定时器的同步。
 *      - 多个定时器可选同一同步信号做同步，也可选不同的同步信号做同步，每个MCPWM单元有3个可供配置的同步信号
 *      - 同步信号的GPIO要连接外部信号输入，不能空置，否则复位后会导致不能同步
 *      - 如果没有外部同步信号源，可将同步GPIO连接到任意需要保持同步的有PWM信号输出的管脚
 *      - 例：mcpwm_half_bridge_sync(half_bridge0, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
 *      - 例：mcpwm_half_bridge_sync(half_bridge2, MCPWM_SELECT_SYNC0, 200, GPIO_MC0_SYNC0_IN);
 * 
 * @param  half_bridge_num 半桥驱动序号，half_bridge0~5
 * @param  sync_sig 同步信号选择，MCPWM_SELECT_SYNC0 / MCPWM_SELECT_SYNC1 / MCPWM_SELECT_SYNC2
 * @param  phase_val 相位左移千分比，单位千分之，0~1000。
 * @param  sync_gpio_num 同步触发的外部GPIO引脚。（输入GPIO）
 */
void mcpwm_half_bridge_sync(half_bridge_t half_bridge_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val, int sync_gpio_num);

/**
 * @brief  半桥PWM占空比输出(用户层用)
 * 
 * @param  half_bridge_num 半桥驱动序号，half_bridge0~5
 * @param  PWMxA_duty PWMA通道输出的占空比，单位百分之（float类型，可设置为小数）
 * @param  PWMxB_duty PWMB通道输出的占空比，单位百分之（float类型，可设置为小数）
 */
void mcpwm_half_bridge_output(half_bridge_t half_bridge_num, float PWMxA_duty, float PWMxB_duty);

/**
 * @brief  半桥相位调整(用户层用)
 * 
 * @param  half_bridge_num 半桥驱动序号，half_bridge0~5
 * @param  sync_sig 同步信号选择，MCPWM_SELECT_SYNC0 / MCPWM_SELECT_SYNC1 / MCPWM_SELECT_SYNC2
 * @param  phase_val 相位左移千分比，单位千分之，0~1000。
 */
void mcpwm_half_bridge_change_phase(half_bridge_t half_bridge_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val);


#endif
