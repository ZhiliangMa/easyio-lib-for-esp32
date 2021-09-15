#include "mcpwm_servo.h"

//注意：默认舵机Servo0~5，使用MCPWM0，如有其他功能占用冲突，请修改.h中宏定义。
//注意：默认舵机Servo6~11，使用MCPWM1，如有其他功能占用冲突，请修改.h中宏定义。
static void mcpwm_servo_output_gpio_init(servo_unit_t servo_unit_num, int PWMxA_gpio_num, int PWMxB_gpio_num)
{
    if(servo_unit_num<3) {
        //初始化 MCPWM0 - PWM0A/PWM0B 的GPIO管脚，并绑定PWMxA/PWMxB信号
        mcpwm_gpio_init(SERVO_0_5_MCPWM, servo_unit_num%3*2, PWMxA_gpio_num);
        mcpwm_gpio_init(SERVO_0_5_MCPWM, servo_unit_num%3*2+1, PWMxB_gpio_num);
    }else{
        //初始化 MCPWM1 - PWM0A/PWM0B 的GPIO管脚，并绑定PWMxA/PWMxB信号
        mcpwm_gpio_init(SERVO_6_11_MCPWM, servo_unit_num%3*2, PWMxA_gpio_num);
        mcpwm_gpio_init(SERVO_6_11_MCPWM, servo_unit_num%3*2+1, PWMxB_gpio_num);
    }
}

static void mcpwm_servo_configuration(servo_unit_t servo_unit_num, uint16_t frequency)
{
    if(servo_unit_num<3) {
        //配置MCPWM0单元的定时器和操作器
        mcpwm_configuration(SERVO_0_5_MCPWM, servo_unit_num%3, frequency);
    }else{
        //配置MCPWM1单元的定时器和操作器
        mcpwm_configuration(SERVO_6_11_MCPWM, servo_unit_num%3, frequency);
    }
}

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
void mcpwm_servo_init(servo_unit_t servo_unit_num, uint16_t frequency, int PWMxA_gpio_num, int PWMxB_gpio_num)
{
    //配置MCPWMx - Timerx的GPIO输出管脚
    mcpwm_servo_output_gpio_init(servo_unit_num, PWMxA_gpio_num, PWMxB_gpio_num);
    //配置MCPWMx单元的定时器和操作器，PWM=50Hz
    mcpwm_servo_configuration(servo_unit_num, frequency);
}

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
void mcpwm_servo_sync(servo_unit_t servo_unit_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val, int sync_gpio_num)
{
    if(servo_unit_num<3) {
        //用内部信号做同步的方法未找到。必须占用一个外部引脚，而且为了避免输入干扰信号，需要配置为下拉
        //同步信号的GPIO要连接 信号，一般为第一个Motor的输出，不能空置，否则复位后会导致不能同步
        mcpwm_gpio_init(SERVO_0_5_MCPWM, sync_sig+2, sync_gpio_num);   //SYNCx
        gpio_pulldown_en(sync_gpio_num);   //Enable pull down on SYNC0  signal
        //使能同步，TIMER_x与MCPWM_SELECT_SYNCx信号同步
        mcpwm_sync_enable(SERVO_0_5_MCPWM, servo_unit_num%3, sync_sig, phase_val);
    }else{
        mcpwm_gpio_init(SERVO_6_11_MCPWM, sync_sig+2, sync_gpio_num);   //SYNCx
        gpio_pulldown_en(sync_gpio_num);   //Enable pull down on SYNC0  signal
        mcpwm_sync_enable(SERVO_6_11_MCPWM, servo_unit_num%3, sync_sig, phase_val);
    }
}

/**
 * @brief Use this function to calcute pulse width for per degree rotation
 *
 * @param  degree_of_rotation the angle in degree to which servo has to rotate
 *
 * @return
 *     - calculated pulse width
 */
static uint32_t servo_per_degree_init(float degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

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
void servo_degree(servo_t channel, float angle)
{
    uint32_t count = 0;
    count = servo_per_degree_init(angle);

    if(channel<6) {
        mcpwm_set_duty_in_us(SERVO_0_5_MCPWM, channel/2%3, channel%2, count);
    }else{
        mcpwm_set_duty_in_us(SERVO_6_11_MCPWM, channel/2%3, channel%2, count);
    }
}
