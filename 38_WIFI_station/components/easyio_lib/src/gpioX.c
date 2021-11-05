
#include "gpioX.h"

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
void gpiox_set_ppOutput(uint32_t gpio_num, uint32_t level)
{
    gpio_pad_select_gpio(gpio_num);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);
    /* GPIO set output level */
    gpio_set_level(gpio_num, level);
}

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
void gpiox_set_input(uint32_t gpio_num, uint32_t pullup_en, uint32_t pulldown_en)
{
    gpio_config_t io_conf;
    //IO中断类型：禁止中断
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //IO模式：输入
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19 配置GPIO_OUT寄存器
    io_conf.pin_bit_mask = (1ULL<<gpio_num);
    //下拉电阻：禁止
    io_conf.pull_down_en = pulldown_en;
    //上拉电阻：禁止
    io_conf.pull_up_en = pullup_en;
    //使用给定设置配置GPIO
    gpio_config(&io_conf);
}

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
void gpiox_set_intr_input(uint32_t gpio_num, uint32_t pullup_en, uint32_t pulldown_en, uint32_t intr_type, void *isr_handler)
{
    gpio_config_t io_conf;
    //IO中断类型：intr_type
    io_conf.intr_type = intr_type;
    //IO模式：输入
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19 配置GPIO_OUT寄存器
    io_conf.pin_bit_mask = (1ULL<<gpio_num);
    //下拉电阻：禁止
    io_conf.pull_down_en = pulldown_en;
    //上拉电阻：禁止
    io_conf.pull_up_en = pullup_en;
    //使用给定设置配置GPIO
    gpio_config(&io_conf);

    //安装GPIO ISR处理程序服务。为所有GPIO中断注册一个全局ISR，并通过gpio_isr_handler_add（）函数注册各个引脚处理程序。
    gpio_install_isr_service(0);
    //添加中断回调处理函数
    gpio_isr_handler_add(gpio_num, isr_handler, (void*) gpio_num);//gpio_isr_handler
}
