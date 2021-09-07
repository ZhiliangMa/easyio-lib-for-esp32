#ifndef __LEDC_PWM_H__
#define __LEDC_PWM_H__

#include "driver/ledc.h"
#include "math.h"

//RGB LED的配置
//Timer0、ledc高速通道、CH0-GPIO33、CH1-GPIO19、CH2-GPIO23
#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (33) //高速通道-CH0的Pin
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0 //高速通道-CH0的ledc通道
#define LEDC_HS_CH1_GPIO       (19) //高速通道-CH1的Pin
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1 //高速通道-CH1的ledc通道
#define LEDC_HS_CH2_GPIO       (23) //高速通道-CH2的Pin
#define LEDC_HS_CH2_CHANNEL    LEDC_CHANNEL_2 //高速通道-CH2的ledc通道
#define LEDC_HS_CH_NUM         (3) //高速通道的总数目，ESP32可设置的最大值为8

//单色LED的配置
//Timer1、ledc低速通道、CH0-GPIO5
#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH0_GPIO       (5) //低速通道-CH0的Pin
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0 //低速通道-CH0的ledc通道
#define LEDC_LS_CH_NUM         (1) //低速通道的总数目，ESP32可设置的最大值为8

#define LEDC_FREQ_HZ           (5000) //ledc输出频率，Hz
#define LEDC_TIMER_BIT         LEDC_TIMER_13_BIT //定时器的位数，决定了输出PWM的分辨率

#define LEDC_MAX_DUTY          (int)(pow(2, LEDC_TIMER_BIT)-1) //根据定时器位数，计算出可用分辨率的最大值
#define LEDC_HALF_DUTY         LEDC_MAX_DUTY/2
#define LEDC_TEST_FADE_TIME    (3000) //例程中的渐变过程时间，单位ms


//ledc高速通道的结构体数组
extern ledc_channel_config_t ledc_hs_ch[LEDC_HS_CH_NUM];
//ledc低速通道的结构体数组
extern ledc_channel_config_t ledc_ls_ch[LEDC_LS_CH_NUM];

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
void ledc_pwm_hs_init(void);

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
void ledc_pwm_ls_init(void);

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
void ledc_pwm_set_fade(ledc_channel_config_t ledc_hs[], uint32_t ch, uint32_t duty, uint32_t time);

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
void ledc_pwm_set_duty(ledc_channel_config_t ledc_hs[], uint32_t ch, uint32_t duty);

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
void ledc_pwm_set_fade_duty_cycle(ledc_channel_config_t ledc_hs[], uint32_t ch, uint32_t duty_cycle, uint32_t time);

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
void ledc_pwm_set_duty_cycle(ledc_channel_config_t ledc_hs[], uint32_t ch, uint32_t duty_cycle);

#endif
