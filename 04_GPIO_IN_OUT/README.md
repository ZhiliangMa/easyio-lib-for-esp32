# 04_GPIO_IN_OUT

## 例程简介

- 使用 `gpioX.c.h` 驱动模块，对ESP32的 `GPIO` 进行输入输出设置

- **点LED灯** 演示 `GPIO_OUT`

- **读按键的电平值**  演示 `GPIO_IN`


## 硬件连接

|        | LED    | KEY   |
| ------ | ------ | ----- |
| ESP32  | GPIO33 | GPIO0 |

ESP32 的 34 个物理 GPIO pad 的序列号为：0-­19, 21­-23, 25-­27, 32-­39。其中 GPIO 34­-39 仅用作输入管脚，其他的既可以作为输入又可以作为输出管脚。

ESP32的GPIO信号源，可以从 `GPIO交换矩阵` 和 `IO_MUX` 中任意选择。

其中，`GPIO交换矩阵` 几乎支持任意IO的映射，极大提高了硬件设计的灵活性。但对最大输入输出速率有限制，信号速率不能超过`30MHz`。常用于 普通GPIO、MCPWM、LEDc、RMT、UART、I2C 等低速信号的输入输出。

而 `IO_MUX` 则支持上至80MHz的最大速率，但不支持任意IO映射，只能选定特定的端口。常用于 SPI、SDIO、以太网 等高速信号的输出。

另外， 选用 `IO_MUX` 可以实现更好的高频数字特性。

`GPIO交换矩阵` 和 `IO_MUX` 的详细介绍，请自行查阅：[ESP32 技术参考手册 (PDF)](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_cn.pdf)


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