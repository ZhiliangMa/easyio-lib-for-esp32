#ifndef __LED_H
#define __LED_H

#include "sdkconfig.h"
#include "driver/gpio.h"

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

/**
 * @brief  led初始化，设置推挽输出，设置初始电平
 *      - 例：led_init(5, 0);
 * 
 * @param  led_io_num led的GPIO端口号
 * @param  level 端口电平值。0/1
 * 
 * @return
 *     - none
 */
void led_init(gpio_num_t led_io_num, uint32_t level);

/**
 * @brief  led亮，输出为高电平
 *      - 例：led_on(5);
 * 
 * @param  led_io_num led的GPIO端口号
 * 
 * @return
 *     - none
 */
void led_on(gpio_num_t led_io_num);

/**
 * @brief  led灭，输出为低电平
 *      - 例：led_off(5);
 * 
 * @param  led_io_num led的GPIO端口号
 * 
 * @return
 *     - none
 */
void led_off(gpio_num_t led_io_num);

/**
 * @brief  设置led端口电平值
 *      - 例：led_set(5, 0);
 * 
 * @param  led_io_num led的GPIO端口号
 * @param  level 端口电平值。0/1
 * 
 * @return
 *     - none
 */
void led_set(gpio_num_t led_io_num, uint32_t level);

/**
 * @brief  led闪烁，亮灭状态发生一次变化
 *      - 例：led_blink(5);
 * 
 * @param  led_io_num led的GPIO端口号
 * 
 * @return
 *     - none
 */
void led_blink(gpio_num_t led_io_num);

#endif
