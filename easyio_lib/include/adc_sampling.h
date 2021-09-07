#ifndef __ADC_SAMPLING_H__
#define __ADC_SAMPLING_H__

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <stdarg.h>


#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate

//adc初始化调试信息输出开关
#define adc_Dlog        0


/**
 * @brief  ADC1及输入通道初始化（在特定衰减下表征ADC的特性，并生成ADC电压曲线）
 *      - 支持函数重载，支持输入不定数目的通道参数，ESP32-ADC1通道数目最大值为8，总参数数目为 3~10。
 *        例：
 *          adc1_init_with_calibrate(ADC_ATTEN_DB_0, 1, ADC_CHANNEL_6);
 *          adc1_init_with_calibrate(ADC_ATTEN_DB_11, 1, ADC_CHANNEL_6);
 *          adc1_init_with_calibrate(ADC_ATTEN_DB_11, 2, ADC_CHANNEL_6, ADC_CHANNEL_7);
 *      - 由于ADC2不能与WIFI共用，所以尽量优先使用ADC1，且ADC2的读取方式与ADC1不同，也就没有在esayIO中提供ADC2的初始化和读取函数
 *      - 默认设置ADC转换数据宽度为12Bit，0~4095。（几乎不需要更改，需要更改的情况一般是改为8Bit来缩小RAM占用。但ESP32的ADC不支持8Bit，最低9Bit）
 * 
 * @param  atten    输入通道衰减系数（决定了输入电压量程）。有 0、2.5、6、11DB 可选，详见上表
 * @param  ch_num   总ADC1输入通道 的数量。channel 与 Pin 对照关系，详见上表
 * @param  (...)    ADC1输入通道列表。支持不定数目参数，数目为 1~8。值为 ADC_CHANNEL_0, ADC_CHANNEL_1... ADC_CHANNEL_7。
 * 
 * @return
 *      - none
 * 
 */
void adc1_init_with_calibrate(adc_atten_t atten, int ch_num, ...);

/**
 * @brief  获取ADC1通道x经校准补偿后的转换电压，单位mV
 *      - 注意：使用该函数前，一定要用 adc1_init_with_calibrate 对ADC1进行校准并初始化，否则会报错
 * 
 * @param  channel 要读取的ADC1输入通道
 * 
 * @return
 *     - 电压值，单位mV。uint32_t类型
 * 
 */
uint32_t adc1_cal_get_voltage(adc_channel_t channel);

/**
 * @brief  获取ADC1通道x，经多重采样平均后，并校准补偿后的转换电压，单位mV
 *      - 注意：使用该函数前，一定要用 adc1_init_with_calibrate 对ADC1进行校准并初始化，否则会报错
 * 
 * @param  channel 要读取的ADC1输入通道
 * @param  mul_num 多重采样的次数
 * 
 * @return
 *     - 电压值，单位mV。uint32_t类型
 * 
 */
uint32_t adc1_cal_get_voltage_mul(adc_channel_t channel, uint32_t mul_num);


#endif
