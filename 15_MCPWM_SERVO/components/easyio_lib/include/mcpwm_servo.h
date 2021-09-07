#ifndef __MCPWM_SERVO_H__
#define __MCPWM_SERVO_H__

#include "mcpwm_motor.h"
#include "easyio_mcpwm_config.h"


/**
 * @brief 可选择的舵机单元编号，共6个。一次性初始化一个单元，包含两路舵机。
 */
typedef enum {
    servo_0_1_unit = 0,
    servo_2_3_unit,
    servo_4_5_unit,
    servo_6_7_unit,
    servo_8_9_unit,
    servo_10_11_unit,
    servo_unit_max,      /*!<Num of servo_unit on easyIO*/
} servo_unit_t;

/**
 * @brief 可选择的舵机序号，共12个。从中选取要控制的舵机。
 */
typedef enum {
    servo0 = 0,     /*!<servo0 selected*/
    servo1,         /*!<servo1 selected*/
    servo2,         /*!<servo2 selected*/
    servo3,         /*!<servo3 selected*/
    servo4,         /*!<servo4 selected*/
    servo5,         /*!<servo5 selected*/
    servo6,         /*!<servo6 selected*/
    servo7,         /*!<servo7 selected*/
    servo8,         /*!<servo8 selected*/
    servo9,         /*!<servo9 selected*/
    servo10,        /*!<servo10 selected*/
    servo11,        /*!<servo11 selected*/
    servo_max,      /*!<Num of Servo on easyIO*/
} servo_t;

/**
 * @brief  Servo舵机配置(用户层用)
 *      - 每次指定舵机单元、PWM频率 和 2个PWM引脚，供2路舵机使用。（如需多路舵机相位相等，可用"mcpwm_servo_sync"开启相位同步）
 *      - 舵机信号可映射到任意GPIO输出管脚。ESP32：0-19, 21-23, 25-27, 32-39。GPIO 34-39 仅用作输入管脚。
 *      - 例：mcpwm_servo_init(servo_0_1_unit, 50, GPIO_SERVO0_OUT, GPIO_SERVO1_OUT);
 *      - 例：mcpwm_servo_init(servo_2_3_unit, 50, GPIO_SERVO2_OUT, GPIO_SERVO3_OUT);
 * 
 * @param  servo_unit_num 舵机单元，servo_0_1_unit = 0/servo_2_3_unit/servo_4_5_unit/servo_6_7_unit/servo_8_9_unit/servo_10_11_unit
 * @param  frequency PWM频率，单位Hz
 * @param  PWMxA_gpio_num 舵机A管脚Pin
 * @param  PWMxB_gpio_num 舵机B管脚Pin
 * 
 * @return
 *     - none
 * 
 */
void mcpwm_servo_init(servo_unit_t servo_unit_num, uint16_t frequency, int PWMxA_gpio_num, int PWMxB_gpio_num);

/**
 * @brief  舵机PWM相位同步配置(用户层用)
 *      - 每次指定舵机序号、同步信号、相移千分比、同步触发的外部GPIO引脚，来启动对应定时器的同步。
 *      - 多个定时器可选同一同步信号做同步，也可选不同的同步信号做同步，每个MCPWM单元有3个可供配置的同步信号
 *      - 同步信号的GPIO要连接外部信号输入，不能空置，否则复位后会导致不能同步
 *      - 如果没有外部同步信号源，可将同步GPIO连接到任意需要保持同步的有PWM信号输出的管脚
 *      - 例：mcpwm_servo_sync(servo_0_1_unit, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
 *      - 例：mcpwm_servo_sync(servo_2_3_unit, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
 * 
 * @param  servo_unit_num 舵机单元，servo_0_1_unit = 0/servo_2_3_unit/servo_4_5_unit/servo_6_7_unit/servo_8_9_unit/servo_10_11_unit
 * @param  sync_sig 同步信号选择，MCPWM_SELECT_SYNC0 / MCPWM_SELECT_SYNC1 / MCPWM_SELECT_SYNC2
 * @param  phase_val 相位左移千分比，单位千分之，0~1000。
 * @param  sync_gpio_num 同步触发的外部GPIO引脚。（输入GPIO）
 * 
 * @return
 *     - none
 * 
 */
void mcpwm_servo_sync(servo_unit_t servo_unit_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val, int sync_gpio_num);

/**
 * @brief  控制舵机的角度(用户层用)(默认舵机的角度为90°，修改在.h文件中宏定义的SERVO_MAX_DEGREE)
 * 
 * @param  channel 舵机通道，servo0~11
 * @param  angle 控制的角度（浮点型，可为小数）
 * 
 * @return
 *     - none
 * 
 */
void servo_degree(servo_t channel, float angle);


#endif
