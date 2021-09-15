#ifndef __GPIOX_H__
#define __GPIOX_H__

#include "driver/gpio.h"

//gpiox_set_level(uint32_t gpio_num, uint32_t level)   gpio_set_level(uint32_t gpio_num, uint32_t level)
#define gpiox_set_level gpio_set_level
#define gpiox_get_level gpio_get_level

/**
 * @brief  GPIO配置为推挽输出模式。并对 引脚序号、初始电平 进行配置。
 * 
 * @param  gpio_num GPIO管脚编号，0~19、21~23、25~27、32~33
 *                              （ESP32只有28个GPIO可用于输出）
 * @param  level    GPIO初始电平，0/1
 * 
 * @return
 *     - none
 */
void gpiox_set_ppOutput(uint32_t gpio_num, uint32_t level);

/**
 * @brief  GPIO配置为输入模式。并对 引脚序号、上拉方式、下拉方式 进行配置。
 * 
 * @param  gpio_num GPIO管脚编号，0~19、21~23、25~27、32~39
 *                  （ESP32共有34个GPIO，其中 34~39 仅用作输入）
 * @param  pullup_en    上拉电阻使能与否，0/1
 * @param  pulldown_en  下拉电阻使能与否，0/1
 * 
 * @return
 *     - none
 */
void gpiox_set_input(uint32_t gpio_num, uint32_t pullup_en, uint32_t pulldown_en);

/**
 * @brief  GPIO配置为中断输入模式。并对 引脚序号、中断类型、上拉方式、下拉方式、isr中断服务函数 进行配置。
 * 
 * @param  gpio_num GPIO管脚编号，0~19、21~23、25~27、32~39
 *                  （ESP32共有34个GPIO，其中 34~39 仅用作输入）
 * @param  pullup_en    上拉电阻使能与否，0/1
 * @param  pulldown_en  下拉电阻使能与否，0/1
 * @param  intr_type    中断触发类型
 *                      - GPIO_PIN_INTR_POSEDGE  上升沿中断
 *                      - GPIO_PIN_INTR_NEGEDGE  下降沿中断
 *                      - GPIO_PIN_INTR_ANYEDGE  双边沿中断
 *                      - GPIO_PIN_INTR_LOLEVEL  低电平中断
 *                      - GPIO_PIN_INTR_HILEVEL  高电平中断
 * @param  isr_handler  ISR中断处理服务函数
 * 
 * @return
 *     - none
 */
void gpiox_set_intr_input(uint32_t gpio_num, uint32_t pullup_en, uint32_t pulldown_en, uint32_t intr_type, void *isr_handler);

#endif
