# 04_GPIO_IN_OUT

## 例程简介

- 使用 `gpioX.c.h` 驱动模块，对ESP32的 `GPIO` 进行输入输出设置

- **点LED灯** 演示 `GPIO_OUT`

- **读按键的电平值**  演示 `GPIO_IN`



## 关键函数

```c
// GPIO配置为推挽输出模式。并对 引脚序号、初始电平 进行配置
void gpiox_set_ppOutput(uint32_t gpio_num, uint32_t level);

// GPIO配置为输入模式。并对 引脚序号、上拉方式、下拉方式 进行配置
void gpiox_set_input(uint32_t gpio_num, uint32_t pullup_en, uint32_t pulldown_en);

// GPIO配置为中断输入模式。并对 引脚序号、中断类型、上拉方式、下拉方式、isr中断服务函数 进行配置。
void gpiox_set_intr_input(uint32_t gpio_num, uint32_t pullup_en, uint32_t pulldown_en, uint32_t intr_type, void *isr_handler);

// 设置GPIO输出电平值
gpiox_set_level(gpio_num_t gpio_num, uint32_t level);

// 获取GPIO的电平值
gpiox_get_level(gpio_num_t gpio_num, uint32_t level);
```


## 注意事项

- ESP32共有34个GPIO，序号为：0~19、21~23、25~27、32~39。（其中 34~39 仅能用作输入，ESP32只有28个GPIO可用于输出）