# 03_GPIO_IN_OUT

## 例程简介

- 从本章Demo以后，正式使用 `Easyio` 库。以便在原有 `ESP-IDF` 框架和底层驱动库上，提供类似于`Arduino`的驱动库。通过便捷的应用接口，加强底层驱动和外设应用的联系，方便二次开发。

- 使用 `led.c.h` 驱动模块，控制连接在 `GPIO` 上的 `LED`，实现 亮、灭、闪烁 操作。

- `BLINK_GPIO`引脚 可通过 `menuconfig` 或者 VSCode插件的 `GUI Configuration tool` 进行设置。


## 关键函数

```c
// led初始化，设置推挽输出，设置初始电平
void led_init(gpio_num_t led_io_num, uint32_t level);

// led亮，输出为高电平
void led_on(gpio_num_t led_io_num);

// led灭，输出为低电平
void led_off(gpio_num_t led_io_num);

// 设置led端口电平值
void led_set(gpio_num_t led_io_num, uint32_t level);

// led闪烁，亮灭状态发生一次变化
void led_blink(gpio_num_t led_io_num);
```


## 注意事项

- ESP32共有34个GPIO，序号为：0~19、21~23、25~27、32~39。（其中 34~39 仅能用作输入，ESP32只有28个GPIO可用于输出）

- 驱动LED只能使用输出引脚。

- 驱动LED输出的同时，还可使用 `gpio_get_level(gpio_num_t gpio_num);` 获取LED的亮灭状态。