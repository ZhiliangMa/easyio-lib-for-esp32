#include "mcpwm_motor.h"


//注意：默认DC直流有刷电机0~2，使用MCPWM0，如有其他功能占用冲突，请修改.h中宏定义。
//注意：默认DC直流有刷电机3~5，使用MCPWM1，如有其他功能占用冲突，请修改.h中宏定义。
static void mcpwm_dc_motor_output_gpio_init(dc_motor_t motor_num, int PWMxA_gpio_num, int PWMxB_gpio_num)
{
    if(motor_num<3) {
        //初始化 MCPWM0 - PWM0A/PWM0B 的GPIO管脚，并绑定PWMxA/PWMxB信号
        mcpwm_gpio_init(DC_MOTOR_0_2_MCPWM, motor_num%3*2, PWMxA_gpio_num);
        mcpwm_gpio_init(DC_MOTOR_0_2_MCPWM, motor_num%3*2+1, PWMxB_gpio_num);
    }else{
        //初始化 MCPWM1 - PWM0A/PWM0B 的GPIO管脚，并绑定PWMxA/PWMxB信号
        mcpwm_gpio_init(DC_MOTOR_3_5_MCPWM, motor_num%3*2, PWMxA_gpio_num);
        mcpwm_gpio_init(DC_MOTOR_3_5_MCPWM, motor_num%3*2+1, PWMxB_gpio_num);
    }
}

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
void mcpwm_configuration(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, uint16_t frequency)
{
    mcpwm_config_t pwm_config;
    pwm_config.frequency = frequency;    //frequency = xxHz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(mcpwm_num, timer_num, &pwm_config);    //Configure PWM0A & PWM0B with above settings
}

//注意：//默认DC直流有刷电机0~2，使用MCPWM0，如有其他功能占用冲突，请修改.h中宏定义。
//注意：//默认DC直流有刷电机3~5，使用MCPWM1，如有其他功能占用冲突，请修改.h中宏定义。
static void mcpwm_dc_motor_configuration(dc_motor_t motor_num, uint16_t frequency)
{
    if(motor_num<3) {
        //配置MCPWM0单元的定时器和操作器
        mcpwm_configuration(DC_MOTOR_0_2_MCPWM, motor_num%3, frequency);
    }else{
        //配置MCPWM1单元的定时器和操作器
        mcpwm_configuration(DC_MOTOR_3_5_MCPWM, motor_num%3, frequency);
    }
}


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
void mcpwm_dc_motor_init(dc_motor_t motor_num, uint16_t frequency, int PWMxA_gpio_num, int PWMxB_gpio_num)
{
    //配置MCPWMx - Timerx的GPIO输出管脚
    mcpwm_dc_motor_output_gpio_init(motor_num, PWMxA_gpio_num, PWMxB_gpio_num);
    //配置MCPWMx单元的定时器和操作器
    mcpwm_dc_motor_configuration(motor_num, frequency);
    
}

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
void mcpwm_dc_motor_sync(dc_motor_t motor_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val, int sync_gpio_num)
{
    if(motor_num<3) {
        //用内部信号做同步的方法未找到。必须占用一个外部引脚，而且为了避免输入干扰信号，需要配置为下拉
        //同步信号的GPIO要连接 信号，一般为第一个Motor的输出，不能空置，否则复位后会导致不能同步
        mcpwm_gpio_init(DC_MOTOR_0_2_MCPWM, sync_sig+2, sync_gpio_num);   //SYNCx
        gpio_pulldown_en(sync_gpio_num);   //Enable pull down on SYNC0  signal
        //使能同步，TIMER_x与MCPWM_SELECT_SYNCx信号同步
        mcpwm_sync_enable(DC_MOTOR_0_2_MCPWM, motor_num%3, sync_sig, phase_val);
    }else{
        mcpwm_gpio_init(DC_MOTOR_3_5_MCPWM, sync_sig+2, sync_gpio_num);   //SYNCx
        gpio_pulldown_en(sync_gpio_num);   //Enable pull down on SYNC0  signal
        mcpwm_sync_enable(DC_MOTOR_3_5_MCPWM, motor_num%3, sync_sig, phase_val);
    }
}

/**
 * @brief  电机正转，占空比 = duty_cycle %(用户层用)
 * 
 * @param  motor_num 电机序号，motor0~5
 * @param  duty_cycle 占空比，单位百分比，0~100（float浮点）
 * 
 * @return
 *     - none
 */
void dc_motor_forward(dc_motor_t motor_num, float duty_cycle)
{
    if(motor_num<3) {
        mcpwm_set_signal_low(DC_MOTOR_0_2_MCPWM, motor_num%3, MCPWM_OPR_B);
        mcpwm_set_duty(DC_MOTOR_0_2_MCPWM, motor_num%3, MCPWM_OPR_A, duty_cycle);
        mcpwm_set_duty_type(DC_MOTOR_0_2_MCPWM, motor_num%3, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
    }else{
        mcpwm_set_signal_low(DC_MOTOR_3_5_MCPWM, motor_num%3, MCPWM_OPR_B);
        mcpwm_set_duty(DC_MOTOR_3_5_MCPWM, motor_num%3, MCPWM_OPR_A, duty_cycle);
        mcpwm_set_duty_type(DC_MOTOR_3_5_MCPWM, motor_num%3, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
    }
}

/**
 * @brief  电机反转，占空比 = duty_cycle %(用户层用)
 * 
 * @param  motor_num 电机序号，motor0~5
 * @param  duty_cycle 占空比，单位百分比，0~100（float浮点）
 * 
 * @return
 *     - none
 */
void dc_motor_backward(dc_motor_t motor_num, float duty_cycle)
{
    if(motor_num<3) {
        mcpwm_set_signal_low(DC_MOTOR_0_2_MCPWM, motor_num%3, MCPWM_OPR_A);
        mcpwm_set_duty(DC_MOTOR_0_2_MCPWM, motor_num%3, MCPWM_OPR_B, duty_cycle);
        mcpwm_set_duty_type(DC_MOTOR_0_2_MCPWM, motor_num%3, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
    }else{
        mcpwm_set_signal_low(DC_MOTOR_3_5_MCPWM, motor_num%3, MCPWM_OPR_A);
        mcpwm_set_duty(DC_MOTOR_3_5_MCPWM, motor_num%3, MCPWM_OPR_B, duty_cycle);
        mcpwm_set_duty_type(DC_MOTOR_3_5_MCPWM, motor_num%3, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
    }
}

/**
 * @brief 电机停止
 */
void dc_motor_stop(dc_motor_t motor_num)
{
    if(motor_num<3) {
        mcpwm_set_signal_low(DC_MOTOR_0_2_MCPWM, motor_num%3, MCPWM_OPR_A);
        mcpwm_set_signal_low(DC_MOTOR_0_2_MCPWM, motor_num%3, MCPWM_OPR_B);
    }else{
        mcpwm_set_signal_low(DC_MOTOR_3_5_MCPWM, motor_num%3, MCPWM_OPR_A);
        mcpwm_set_signal_low(DC_MOTOR_3_5_MCPWM, motor_num%3, MCPWM_OPR_B);
    }
}
