# 05_Task

## 例程简介

熟悉 `FreeRTOS` 的 `Task` 使用，实现多任务编程。

使用 `FreeRTOS` 的 `Task`，创建两个任务：

- 1. **led_task** 任务，控制LED闪烁

- 2. **gpio_in_task**  任务，串口输出按键电平值


## 硬件连接

|        | LED    | KEY   |
| ------ | ------ | ----- |
| ESP32  | GPIO33 | GPIO0 |


## 关键函数

```c
// FreeRTOS 创建 Task
BaseType_t xTaskCreate( TaskFunction_t pvTaskCode, const char * const pcName, uint16_t usStackDepth, void *pvParameters, UBaseType_t uxPriority, TaskHandle_t *pxCreatedTask );
```


## 注意事项

- ESP32共有`34`个GPIO，序号为：`0~19、21~23、25~27、32~39`。（其中 `34~39` 仅能用作输入，ESP32只有`28`个`GPIO`可用于输出）

- 如果烧录代码过后，ESP32上电开机后反复重启，请将任务分配的栈空间增大

- 任务栈空间为 configMINIMAL_STACK_SIZE 时，会导致在该任务中不能使用 printf，会导致ESP32反复重启