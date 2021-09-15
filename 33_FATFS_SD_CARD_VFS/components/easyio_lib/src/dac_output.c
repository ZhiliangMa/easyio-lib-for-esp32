#include "dac_output.h"


/*  ESP32 DAC channel 与 Pin 对照表

    +----------+---------------------+-------+--------------+--------------+
    |          |       channel       |  Pin  |     Name     |   Function   |
    +==========+=====================+=======+==============+==============+
    |   ESP32  |    DAC_CHANNEL_1    |  14   |    GPIO25    |    GPIO25    |
    |   DAC    +---------------------+-------+--------------+--------------+
    |          |    DAC_CHANNEL_2    |  15   |    GPIO26    |    GPIO26    |
    +----------+---------------------+-------+--------------+--------------+

*/

/**
 * @brief  DAC输出通道x初始化（channel 与 Pin 对照关系，详见上表）
 * 
 * @param  channel DAC输出通道（channel 与 Pin 对照关系，详见上表）
 * 
 * @return
 *     - none
 * 
 */
void dac_channel_init(dac_channel_t channel)
{
    dac_output_enable(channel);
}

/**
 * @brief  DAC通道x输出电压xxmV（供电电源为3.3V时）
 * 
 * @param  channel DAC输出通道
 * @param  voltage 输出电压值，单位mV，范围 0~3300
 * 
 * @return
 *     - none
 * 
 */
void dac_output_0_3V3_voltage_int_mV(dac_channel_t channel, int voltage)
{
    uint32_t value = voltage *256 /3.3 /1000;
    if(value>255) value=255;
    dac_output_voltage(channel, value);
}

/**
 * @brief  DAC通道x输出电压xxV（供电电源为3.3V时）
 * 
 * @param  channel DAC输出通道
 * @param  voltage 输出电压值，单位V，范围 0~3.3
 * 
 * @return
 *     - none
 * 
 */
void dac_output_0_3V3_voltage_float_V(dac_channel_t channel, float voltage)
{
    uint32_t value = voltage *256 /3.3;
    if(value>255) value=255;
    dac_output_voltage(channel, value);
}
