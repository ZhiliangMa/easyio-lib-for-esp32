#include "mcpwm_half_bridge.h"


//是否需要AB反相互补输出
static void mcpwm_half_bridge_inverted(half_bridge_t half_bridge_num)
{
    if(half_bridge_num<3) {
        mcpwm_set_duty_type(HALF_BRIDGE_0_2_MCPWM, half_bridge_num%3, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //Set PWMxA to duty mode zero
        mcpwm_set_duty_type(HALF_BRIDGE_0_2_MCPWM, half_bridge_num%3, MCPWM_OPR_B, MCPWM_DUTY_MODE_1); //Set PWMxB to duty mode one
    }else{
        mcpwm_set_duty_type(HALF_BRIDGE_3_5_MCPWM, half_bridge_num%3, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //Set PWMxA to duty mode zero
        mcpwm_set_duty_type(HALF_BRIDGE_3_5_MCPWM, half_bridge_num%3, MCPWM_OPR_B, MCPWM_DUTY_MODE_1); //Set PWMxB to duty mode one
    }
}

/*//是否添加死区控制
static void mcpwm_deadtime(void)
{
    //add rising edge delay or falling edge delay. There are 8 different types, each explained in mcpwm_deadtime_type_t in mcpwm.h
    //mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_BYPASS_FED, 100, 0);   //Enable deadtime on PWM2A and PWM2B with red = (100)*100ns on PWM2A
    //mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_BYPASS_RED, 0, 200);        //Enable deadtime on PWM1A and PWM1B with fed = (200)*100ns on PWM1B
    //mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_RED_FED_FROM_PWMXA, 50, 100);  //Enable deadtime on PWM0A and PWM0B with red = (50)*100ns & fed = (100)*100ns on PWM0A and PWM0B generated from PWM0A
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_RED_FED_FROM_PWMXA, 656, 67);  //Enable deadtime on PWM0A and PWM0B with red = (656)*100ns & fed = (67)*100ns on PWM0A and PWM0B generated from PWM0A
    //use mcpwm_deadtime_disable function to disable deadtime on mcpwm timer on which it was enabled
}*/

//注意：默认半桥0~2，使用MCPWM0，如有其他功能占用冲突，请修改.h中宏定义。
//注意：默认半桥3~5，使用MCPWM1，如有其他功能占用冲突，请修改.h中宏定义。
static void mcpwm_half_bridge_output_gpio_init(half_bridge_t half_bridge_num, int PWMxA_gpio_num, int PWMxB_gpio_num)
{
    if(half_bridge_num<3) {
        //初始化 MCPWM0 - PWM0A/PWM0B 的GPIO管脚，并绑定PWMxA/PWMxB信号
        mcpwm_gpio_init(HALF_BRIDGE_0_2_MCPWM, half_bridge_num%3*2, PWMxA_gpio_num);
        mcpwm_gpio_init(HALF_BRIDGE_0_2_MCPWM, half_bridge_num%3*2+1, PWMxB_gpio_num);
    }else{
        //初始化 MCPWM1 - PWM0A/PWM0B 的GPIO管脚，并绑定PWMxA/PWMxB信号
        mcpwm_gpio_init(HALF_BRIDGE_3_5_MCPWM, half_bridge_num%3*2, PWMxA_gpio_num);
        mcpwm_gpio_init(HALF_BRIDGE_3_5_MCPWM, half_bridge_num%3*2+1, PWMxB_gpio_num);
    }
}

static void mcpwm_half_bridge_configuration(half_bridge_t half_bridge_num, uint16_t frequency)
{
    if(half_bridge_num<3) {
        //配置MCPWM0单元的定时器和操作器
        mcpwm_configuration(HALF_BRIDGE_0_2_MCPWM, half_bridge_num%3, frequency);
    }else{
        //配置MCPWM1单元的定时器和操作器
        mcpwm_configuration(HALF_BRIDGE_3_5_MCPWM, half_bridge_num%3, frequency);
    }
}

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
void mcpwm_half_bridge_init(half_bridge_t half_bridge_num, uint16_t frequency, output_type_t output_type, int PWMxA_gpio_num, int PWMxB_gpio_num)
{
    //配置MCPWM单元的输出GPIO管脚
    mcpwm_half_bridge_output_gpio_init(half_bridge_num, PWMxA_gpio_num, PWMxB_gpio_num);
    //配置MCPWM单元的定时器和操作器，及PWM频率
    mcpwm_half_bridge_configuration(half_bridge_num, frequency);
    if(output_type){
        //是否需要AB反相互补输出，需要，AB反相互补输出
        mcpwm_half_bridge_inverted(half_bridge_num);
    }
    //!!死区有Bug，就会破坏反相，一用死区就会破坏反相，只能单独使用，暂记作bug
    //是否添加死区控制？添加，保护半桥。死区没调通。而且死区参数也蛮多，单独API配置比较好
    //mcpwm_deadtime();
}

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
void mcpwm_half_bridge_sync(half_bridge_t half_bridge_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val, int sync_gpio_num)
{
    if(half_bridge_num<3) {
        //用内部信号做同步的方法未找到。必须占用一个外部引脚，而且为了避免输入干扰信号，需要配置为下拉
        //同步信号的GPIO要连接 信号，一般为第一个half_bridge的输出，不能空置，否则复位后会导致不能同步
        mcpwm_gpio_init(HALF_BRIDGE_0_2_MCPWM, sync_sig+2, sync_gpio_num);   //SYNCx
        gpio_pulldown_en(sync_gpio_num);   //Enable pull down on SYNC0  signal
        //使能同步，TIMER_x与MCPWM_SELECT_SYNCx信号同步
        mcpwm_sync_enable(HALF_BRIDGE_0_2_MCPWM, half_bridge_num%3, sync_sig, phase_val);
    }else{
        mcpwm_gpio_init(HALF_BRIDGE_3_5_MCPWM, sync_sig+2, sync_gpio_num);   //SYNCx
        gpio_pulldown_en(sync_gpio_num);   //Enable pull down on SYNC0  signal
        mcpwm_sync_enable(HALF_BRIDGE_3_5_MCPWM, half_bridge_num%3, sync_sig, phase_val);
    }
}

/**
 * @brief  半桥PWM占空比输出(用户层用)
 * 
 * @param  half_bridge_num 半桥驱动序号，half_bridge0~5
 * @param  PWMxA_duty PWMA通道输出的占空比，单位百分之（float类型，可设置为小数）
 * @param  PWMxB_duty PWMB通道输出的占空比，单位百分之（float类型，可设置为小数）
 */
void mcpwm_half_bridge_output(half_bridge_t half_bridge_num, float PWMxA_duty, float PWMxB_duty)
{
    if(half_bridge_num<3) {
        mcpwm_set_duty(HALF_BRIDGE_0_2_MCPWM, half_bridge_num%3, MCPWM_OPR_A, PWMxA_duty);
        mcpwm_set_duty(HALF_BRIDGE_0_2_MCPWM, half_bridge_num%3, MCPWM_OPR_B, PWMxB_duty);
    }else{
        mcpwm_set_duty(HALF_BRIDGE_3_5_MCPWM, half_bridge_num%3, MCPWM_OPR_A, PWMxA_duty);
        mcpwm_set_duty(HALF_BRIDGE_3_5_MCPWM, half_bridge_num%3, MCPWM_OPR_B, PWMxB_duty);
    }
}

/**
 * @brief  半桥相位调整(用户层用)
 * 
 * @param  half_bridge_num 半桥驱动序号，half_bridge0~5
 * @param  sync_sig 同步信号选择，MCPWM_SELECT_SYNC0 / MCPWM_SELECT_SYNC1 / MCPWM_SELECT_SYNC2
 * @param  phase_val 相位左移千分比，单位千分之，0~1000。
 */
void mcpwm_half_bridge_change_phase(half_bridge_t half_bridge_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val)
{
    if(half_bridge_num<3) {
        //mcpwm_timer_change_phase(HALF_BRIDGE_0_2_MCPWM, half_bridge_num%3, uint32_t phase_val);
        mcpwm_sync_enable(HALF_BRIDGE_0_2_MCPWM, half_bridge_num%3, sync_sig, phase_val);
    }else{
        //mcpwm_timer_change_phase(HALF_BRIDGE_3_5_MCPWM, half_bridge_num%3, uint32_t phase_val);
        mcpwm_sync_enable(HALF_BRIDGE_3_5_MCPWM, half_bridge_num%3, sync_sig, phase_val);
    }
}

/**
 * @brief 以下两个载波的驱动无用，仅作调试，实际用途太小
 */
/*//是否开启载波
static void mcpwm_carrier(uint16_t carrier)
{
    if(carrier) {
        //in carrier mode very high frequency carrier signal is generated at mcpwm high level signal
        mcpwm_carrier_config_t chop_config;
        chop_config.carrier_period = 6;         //carrier period = (6 + 1)*800ns
        chop_config.carrier_duty = 3;           //carrier duty = (3)*12.5%
        chop_config.carrier_os_mode = MCPWM_ONESHOT_MODE_EN; //If one shot mode is enabled then set pulse width, if disabled no need to set pulse width
        chop_config.pulse_width_in_os = 3;      //first pulse width = (3 + 1)*carrier_period
        chop_config.carrier_ivt_mode = MCPWM_CARRIER_OUT_IVT_DIS;//MCPWM_CARRIER_OUT_IVT_EN; //output signal inversion enable
        //mcpwm_carrier_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &chop_config);  //Enable carrier on PWM2A and PWM2B with above settings
        mcpwm_carrier_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &chop_config);
    }
}

//载波，实用性不强，没DMA，需要载波可以看RMT
void mcpwm_carrier_output_init(void)
{
    //配置MCPWM单元的输出GPIO管脚
    mcpwm_dc_motor_output_gpio_init();
    //配置MCPWM单元的定时器和操作器，PWM=1000Hz
    mcpwm_dc_motor_configuration(1000);
    //是否需要AB反相互补输出，不需要，同相输出
    mcpwm_inverted(0);
    //是否添加死区控制？不添加
    mcpwm_deadtime(0);
    //载波，开启
    mcpwm_carrier(1);
    //是否定时器开启同步？开启同步，并使各端口输出信号同相位
    mcpwm_sync(1, 0, 0, 0);
}*/
