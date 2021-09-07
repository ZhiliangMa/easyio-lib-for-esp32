# 07_GPIO_INTR_Queue

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建两个任务：

- 1. **led_task** 任务，控制LED闪烁

- 2. **gpio_in_task**  任务，串口输出 GPIO中断次数、按键值、led任务状态。并通过按键控制led运行状态

使用 `gpioX.c.h` 驱动模块，来对ESP32的 `GPIO` 输入中断进行配置

使用 FreeRTOS 的 `Queue 消息队列`，来对ESP32的GPIO中断事件进行处理


## 运行现象

- LED闪烁，按动按键会 暂停/继续 LED的闪烁。

- 按动按键时，串口会输出 `gpio中断的引脚Pin号`、`gpio中断次数`、`led_task 任务的运行状态`


## 学习内容

1. ESP32的 `GPIO中断操作`。

2. `FreeRTOS` 的 `任务管理`，对任务运行状态状态进行控制。暂停/继续任务的运行。

3. `FreeRTOS` 的 `Queue 消息队列`，对突发中断事件进行处理。

4. `FreeRTOS` 的 阻塞/非阻塞概念、队列的操作（类似FIFO）。


## 关键函数

```c
// FreeRTOS 创建队列
xQueueCreate(10, sizeof(uint32_t));

// FreeRTOS 向队列发送消息
xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);

// FreeRTOS 接收队列的消息
xQueueReceive(gpio_evt_queue, &gpio_num, portMAX_DELAY);
```


## 注意事项

- 中断ISR中，`printf` 会导致重启。千万别在ESP32中断处理内用 `printf`。

- `FreeRTOS` 对任务的控制，都通过操作 `任务句柄` 实现，而非任务处理函数。

- `FreeRTOS` 的 `Queue 消息队列`，可以简化突发事件的处理方式，但要合理设置队列深度，以防 `单个任务周期内，突发事件的发生次数，大于队列深度，导致丢失部分消息`。

- 中断中向队列发送消息，只能使用 `xQueueSendFromISR`，而不能用 `xQueueSend`。

- 例程中没添加按键防抖。（08_KEY例程中，会单独封装`key.c.h`，来处理按键事件。支持多按键，支持短按/长按检测）