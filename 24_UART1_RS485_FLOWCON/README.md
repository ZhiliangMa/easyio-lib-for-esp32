# 24_UART1_RS485_FLOWCON

## 例程简介

ESP32的 `UART` 硬件，用`硬件流控`做`RS485`通信。

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **uart_rs485_echo_task** ，配置uart1-txrx，设置`硬件流控`以用于`RS485`通信。发送并接收数据。

外接`RS485`总线设备，观察调试结果。

使用 `uart_config.c.h` 驱动模块，来对ESP32的 `UART` 进行配置。进行UART1-`RS485`总线通信操作。


## 运行现象

* LED闪烁。

* 连接`RS485`总线设备，观察调试输出结果。


## 学习内容

1. ESP32的`UART`配置及用法。

2. `RS485`。

详见乐鑫在线文档：
https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/peripherals/uart.html


## 关键函数

```c
// 配置RS485端口，配置uartx，硬件流控（uartx、波特率、输出端口、队列）
void uart_rs485_init_with_hwfc(uart_port_t uart_num, uint32_t baud_rate, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num, int queue_size, QueueHandle_t *uart_queue);
```


## 注意事项

* ESP32做`RS485`通信，使用`硬件流控`，只需`RTS`引脚（Require To Send，发送请求）

* 本来`ESP-IDF`的Demo中，还有GPS的例程-`nmea0183_parser`。测试时没有运行成功，不知道为什么接收不到GPS数据，待以后有空补全一起合并到工程库中。
