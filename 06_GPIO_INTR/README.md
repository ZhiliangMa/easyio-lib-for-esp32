# 06_GPIO_INTR

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建两个任务：

- 1. **led_task** 任务，控制LED闪烁

- 2. **gpio_in_task**  任务，串口输出按键电平值

使用 `gpioX.c.h` 驱动模块，来对ESP32的 `GPIO` 输入中断进行配置


## 硬件连接

|        | LED    | KEY   |
| ------ | ------ | ----- |
| ESP32  | GPIO33 | GPIO0 |


## 运行现象

- LED闪烁。按动按键会 暂停/继续 LED的闪烁。

- 串口不断输出 `gpio中断次数`、`led_task 任务运行状态`


## 学习内容

1. ESP32的 `GPIO中断操作`

2. `FreeRTOS` 的 `任务管理`，对任务运行状态状态进行控制。暂停/继续任务的运行。


## 关键函数

```c
// GPIO配置为中断输入模式。并对 引脚序号、中断类型、上拉方式、下拉方式、isr中断服务函数 进行配置。
void gpiox_set_intr_input(uint32_t gpio_num, uint32_t pullup_en, uint32_t pulldown_en, uint32_t intr_type, void *isr_handler);

// 用户自定义的GPIO-ISR中断处理函数
void IRAM_ATTR gpio_isr_handler(void *arg);

// FreeRTOS 查询任务运行状态
eTaskGetState(TaskHandle_t);

// FreeRTOS 暂停任务
vTaskSuspend(TaskHandle_t);

// FreeRTOS 继续任务
vTaskResume(TaskHandle_t);
```


## 注意事项

- 中断ISR中，`printf` 会导致重启。千万别在ESP32中断处理内用 `printf`。

- `FreeRTOS` 对任务的控制，都通过操作 `任务句柄` 实现，而非任务处理函数。

- IRAM_ATTR 是将函数定义在iRAM区，乐鑫对IDF代码中段功能的说明：https://blog.csdn.net/espressif/article/details/78563811