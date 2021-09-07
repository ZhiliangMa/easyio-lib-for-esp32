#ifndef __MCPWM_MOTOR_H__
#define __MCPWM_MOTOR_H__

#include "easyio_mcpwm_config.h"


/**
 * @brief 可选择的电机序号，共6个
 */
typedef enum {
    motor0 = 0,     /*!<motor0 selected*/
    motor1,         /*!<motor1 selected*/
    motor2,         /*!<motor2 selected*/
    motor3,         /*!<motor3 selected*/
    motor4,         /*!<motor4 selected*/
    motor5,         /*!<motor5 selected*/
    motor_max,      /*!<Num of DC_MOTOR on easyIO*/
} dc_motor_t;

/**
 * @brief  MCPWM 定时器及操作器配置(用户层接触不到，开发层用)
 * 
 * @param  mcpwm_num MCPWM序号，MCPWM_UNIT_0 / MCPWM_UNIT_1
 * @param  timer_num 定时器序号，MCPWM_TIMER_0 / MCPWM_TIMER_1 / MCPWM_TIMER_2
 * @param  frequency PWM频率，单位Hz
 * 
 * @return
 *     - none
 * 
 */
void mcpwm_configuration(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, uint16_t frequency);

/**
 * @brief  直流有刷电机PWM驱动初始化及配置(用户层用)
 *      - 每次指定PWM频率 和 2个PWM引脚，供1路直流有刷电机使用。（如需多路有刷电机的PWM相位相等，可用"mcpwm_dc_motor_sync"开启相位同步）
 *      - MCPWM的信号可映射到任意GPIO输出管脚。ESP32：0-19, 21-23, 25-27, 32-39。GPIO 34-39 仅用作输入管脚。
 *      - 例：mcpwm_dc_motor_init(motor0, 1000, GPIO_MOTOR0_PWMA_OUT, GPIO_MOTOR0_PWMB_OUT);
 *      - 例：mcpwm_dc_motor_init(motor1, 1000, GPIO_MOTOR1_PWMA_OUT, GPIO_MOTOR1_PWMB_OUT);
 * 
 * @param  motor_num 电机序号，motor0~5
 * @param  frequency PWM频率，单位Hz
 * @param  PWMxA_gpio_num PWMA管脚Pin
 * @param  PWMxB_gpio_num PWMB管脚Pin
 * 
 * @return
 *     - none
 * 
 */
void mcpwm_dc_motor_init(dc_motor_t motor_num, uint16_t frequency, int PWMxA_gpio_num, int PWMxB_gpio_num);

/**
 * @brief  直流有刷电机PWM相位同步配置(用户层用)
 *      - 每次指定电机序号、同步信号、相移千分比、同步触发的外部GPIO引脚，来启动对应定时器的同步。
 *      - 多个定时器可选同一同步信号做同步，也可选不同的同步信号做同步，每个MCPWM单元有3个可供配置的同步信号
 *      - 同步信号的GPIO要连接外部信号输入，不能空置，否则复位后会导致不能同步
 *      - 如果没有外部同步信号源，可将同步GPIO连接到任意需要保持同步的有PWM信号输出的管脚
 *      - 例：mcpwm_dc_motor_sync(motor0, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
 *      - 例：mcpwm_dc_motor_sync(motor2, MCPWM_SELECT_SYNC0, 200, GPIO_MC0_SYNC0_IN);
 * 
 * @param  motor_num 电机序号，motor0~5
 * @param  sync_sig 同步信号选择，MCPWM_SELECT_SYNC0 / MCPWM_SELECT_SYNC1 / MCPWM_SELECT_SYNC2
 * @param  phase_val 相位左移千分比，单位千分之，0~1000。
 * @param  sync_gpio_num 同步触发的外部GPIO引脚。（输入GPIO）
 * 
 * @return
 *     - none
 * 
 */
void mcpwm_dc_motor_sync(dc_motor_t motor_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val, int sync_gpio_num);

/**
 * @brief  电机正转，占空比 = duty_cycle %(用户层用)
 * 
 * @param  motor_num 电机序号，motor0~5
 * @param  duty_cycle 占空比，单位百分比，0~100（float浮点）
 * 
 * @return
 *     - none
 */
void dc_motor_forward(dc_motor_t motor_num, float duty_cycle);

/**
 * @brief  电机反转，占空比 = duty_cycle %(用户层用)
 * 
 * @param  motor_num 电机序号，motor0~5
 * @param  duty_cycle 占空比，单位百分比，0~100（float浮点）
 * 
 * @return
 *     - none
 */
void dc_motor_backward(dc_motor_t motor_num, float duty_cycle);

/**
 * @brief 电机停止
 */
void dc_motor_stop(dc_motor_t motor_num);


#endif
