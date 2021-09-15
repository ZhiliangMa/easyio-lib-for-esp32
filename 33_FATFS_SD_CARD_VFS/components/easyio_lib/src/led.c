
#include "led.h"

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
void led_init(gpio_num_t led_io_num, uint32_t level)
{
    /*// 没有用这种简便的方法，是因为不能在GPIO输出时，获取该引脚电平值。导致不能实现 led_blink
    gpio_pad_select_gpio(led_io_num);
    gpio_set_direction(led_io_num, GPIO_MODE_OUTPUT);
    led_set(led_io_num, level);*/

    gpio_config_t io_conf;
    //IO中断类型：禁止中断
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //IO模式：输入输出
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19 配置GPIO_OUT寄存器
    io_conf.pin_bit_mask = (1ULL<<led_io_num);
    //下拉电阻：禁止
    io_conf.pull_down_en = 0;
    //上拉电阻：禁止
    io_conf.pull_up_en = 0;
    //使用给定设置配置GPIO
    gpio_config(&io_conf);

    led_set(led_io_num, level);
}

/**
 * @brief  led亮，输出为高电平
 *      - 例：led_on(5);
 * 
 * @param  led_io_num led的GPIO端口号
 * 
 * @return
 *     - none
 */
void led_on(gpio_num_t led_io_num)
{
    gpio_set_level(led_io_num, 1);
}

/**
 * @brief  led灭，输出为低电平
 *      - 例：led_off(5);
 * 
 * @param  led_io_num led的GPIO端口号
 * 
 * @return
 *     - none
 */
void led_off(gpio_num_t led_io_num)
{
    gpio_set_level(led_io_num, 0);
}

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
void led_set(gpio_num_t led_io_num, uint32_t level)
{
    gpio_set_level(led_io_num, level);
}

/**
 * @brief  led闪烁，亮灭状态发生一次变化
 *      - 例：led_blink(5);
 * 
 * @param  led_io_num led的GPIO端口号
 * 
 * @return
 *     - none
 */
void led_blink(gpio_num_t led_io_num)
{
    uint8_t level;

    // 读取LED的输出电平状态
    level = gpio_get_level(led_io_num);
    // 取反输出
    gpio_set_level(led_io_num, !level);
}
