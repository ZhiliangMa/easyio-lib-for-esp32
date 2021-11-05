#include "ledc_pwm.h"

//通过选择以下内容为LED控制器的每个通道准备单独的配置：
//-控制器的通道号
//-输出占空比，初始设置为0
//-LED连接到的GPIO编号
//-速度模式，高或低
//-计时器服务选定的通道
//注意：如果不同的通道使用一个计时器，那么这些通道的频率和bit_num将会相同

//ledc高速通道的结构体数组
ledc_channel_config_t ledc_hs_ch[LEDC_HS_CH_NUM] = {
    {
        .channel    = LEDC_HS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },
    {
        .channel    = LEDC_HS_CH1_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH1_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },
    {
        .channel    = LEDC_HS_CH2_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH2_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },
};

//ledc低速通道的结构体数组
ledc_channel_config_t ledc_ls_ch[LEDC_LS_CH_NUM] = {
    {
        .channel    = LEDC_LS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_LS_CH0_GPIO,
        .speed_mode = LEDC_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_LS_TIMER
    },
};

/**
 * @brief  ledc高速通道初始化，使用的通道详见结构体数组（默认CH0-GPIO18，CH1-GPIO19，CH2-GPIO23）
 *         因参数众多，不便于函数传参修改，如需重新设定请更改 结构体数组 和 ledc_pwm.h的宏定义
 *         默认频率为5KHz，占空比0~100%可调节（13Bit分辨率，对应0~8191）
 *         频率参数不建议随意更改，如需更改请移步ledc_pwm.h的宏定义，请遵照手册的参数去配置
 * 
 * @param
 *      - none
 * @return
 *      - none
 * 
 */
void ledc_pwm_hs_init(void)
{
    //准备LEDc的高速通道定时器：Timer0
    ledc_timer_config_t ledc_hs_timer = {
        .duty_resolution = LEDC_TIMER_BIT,    // 定时器位数13Bit，决定PWM占空比的分辨率
        .freq_hz = LEDC_FREQ_HZ,              // PWM频率：5000Hz
        .speed_mode = LEDC_HS_MODE,           // 定时器模式：高速
        .timer_num = LEDC_HS_TIMER,           // 定时器序号：Timer0
        .clk_cfg = LEDC_AUTO_CLK,             // 时钟源：自动选择
    };
    // 设置用于高速通道的timer0的配置
    ledc_timer_config(&ledc_hs_timer);

    // LEDc高速输出通道配置
    for (int ch = 0; ch < LEDC_HS_CH_NUM; ch++) {
        ledc_channel_config(&ledc_hs_ch[ch]);
    }

    // 初始化 fade service.（硬件定时器渐变功能）
    ledc_fade_func_install(0);
}

/**
 * @brief  ledc低速通道初始化，使用的通道详见结构体数组（默认CH0-GPIO5）
 *         因参数众多，不便于函数传参修改，如需重新设定请更改 结构体数组 和 ledc_pwm.h的宏定义
 *         默认频率为5KHz，占空比0~100%可调节（13Bit分辨率，对应0~8191）
 *         频率参数不建议随意更改，如需更改请移步ledc_pwm.h的宏定义，请遵照手册的参数去配置
 * 
 * @param
 *      - none
 * @return
 *      - none
 * 
 */
void ledc_pwm_ls_init(void)
{
    //准备LEDc的低速通道定时器：Timer1
    ledc_timer_config_t ledc_ls_timer = {
        .duty_resolution = LEDC_TIMER_BIT,    // 定时器位数13Bit，决定PWM占空比的分辨率
        .freq_hz = LEDC_FREQ_HZ,              // PWM频率：5000Hz
        .speed_mode = LEDC_LS_MODE,           // 定时器模式：低速
        .timer_num = LEDC_LS_TIMER,           // 定时器序号：Timer1
        .clk_cfg = LEDC_AUTO_CLK,             // 时钟源：自动选择
    };
    // 设置用于低速通道的timer1的配置
    ledc_timer_config(&ledc_ls_timer);

    // LEDc低速输出通道配置
    for (int ch = 0; ch < LEDC_LS_CH_NUM; ch++) {
        ledc_channel_config(&ledc_ls_ch[ch]);
    }

    // 初始化 fade service.（硬件定时器渐变功能）
    ledc_fade_func_install(0);
}

/**
 * @brief  指定高速/低速ledc通道，在设定的时间内，从当前脉宽 渐变到 期望脉宽（0~定时器的满分辨率，需要手动计算）
 * 
 * @param  ledc_hs[]    高速/低速ledc通道的结构体数组
 * @param  ch           操作的通道
 * @param  duty         渐变到的期望脉宽值（与定时器Bit有关，100%占空比对应满分辨率）
 * @param  time         渐变过程的时间，单位ms
 * 
 * @return
 *      - none
 * 
 */
void ledc_pwm_set_fade(ledc_channel_config_t ledc_hs[], uint32_t ch, uint32_t duty, uint32_t time)
{
    ledc_set_fade_with_time(ledc_hs[ch].speed_mode,
                    ledc_hs[ch].channel, duty, time);
    ledc_fade_start(ledc_hs[ch].speed_mode,
            ledc_hs[ch].channel, LEDC_FADE_NO_WAIT);
}

/**
 * @brief  指定高速/低速ledc通道，设定脉冲宽度（0~定时器的满分辨率，需要手动计算）
 * 
 * @param  ledc_hs[]    高速/低速ledc通道的结构体数组
 * @param  ch           操作的通道
 * @param  duty         设定的脉冲宽度值（与定时器Bit有关，100%占空比对应满分辨率）
 * 
 * @return
 *      - none
 * 
 */
void ledc_pwm_set_duty(ledc_channel_config_t ledc_hs[], uint32_t ch, uint32_t duty)
{
    ledc_set_duty(ledc_hs[ch].speed_mode, ledc_hs[ch].channel, duty);
    ledc_update_duty(ledc_hs[ch].speed_mode, ledc_hs[ch].channel);
}

/**
 * @brief  指定高速/低速ledc通道，在设定的时间内，从当前占空比 渐变到 期望占空比（0~1000‰，省去了手动计算，更加方便）
 * 
 * @param  ledc_hs[]    高速/低速ledc通道的结构体数组
 * @param  ch           操作的通道
 * @param  duty_cycle   渐变到的期望占空比（0~1000，单位‰，千分之）
 * @param  time         渐变过程的时间，单位ms
 * 
 * @return
 *      - none
 * 
 */
void ledc_pwm_set_fade_duty_cycle(ledc_channel_config_t ledc_hs[], uint32_t ch, uint32_t duty_cycle, uint32_t time)
{
    if (duty_cycle > 1000) duty_cycle = 1000;
    uint32_t duty = (float)duty_cycle /1000 *LEDC_MAX_DUTY;
    ledc_pwm_set_fade(ledc_hs, ch, duty, time);
}

/**
 * @brief  指定高速/低速ledc通道，设置固定占空比输出（0~1000‰，省去了手动计算，更加方便）
 * 
 * @param  ledc_hs[]    高速/低速ledc通道的结构体数组
 * @param  ch           操作的通道
 * @param  duty         设定的占空比（0~1000，单位‰，千分之）
 * 
 * @return
 *      - none
 * 
 */
void ledc_pwm_set_duty_cycle(ledc_channel_config_t ledc_hs[], uint32_t ch, uint32_t duty_cycle)
{
    if (duty_cycle > 1000) duty_cycle = 1000;
    uint32_t duty = (float)duty_cycle /1000 *LEDC_MAX_DUTY;
    ledc_pwm_set_duty(ledc_hs, ch, duty);
}
