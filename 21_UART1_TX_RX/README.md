# 21_UART1_TX_RX

## 例程简介

ESP32的 `UART` 硬件，用于UART串口通信。

使用 `FreeRTOS` 的 `Task`，创建三个任务：

1. **led_task** ，控制LED闪烁

2. **uart_tx_task** ，配置uart1-txrx，通过串口发送数据。

3. **uart_rx_task** ，uart1-rx接收数据，使用等待时长的方式去接收数据。

硬件上需要将 `uart1-tx` 与 `uart1-tx` 的引脚连接。

使用 `uart_config.c.h` 驱动模块，来对ESP32的 `UART` 进行配置。进行UART串口通信操作。


## 运行现象

* LED闪烁。
 
* 硬件上将 `uart1-tx` 与 `uart1-tx` 的引脚连接。打开调试窗口，可以接收到消息 "uart1, Hello!"。


## 学习内容

1. ESP32的`UART`配置及用法。

详见乐鑫在线文档：
https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/peripherals/uart.html


## 关键函数

```c
// 配置uartx，无硬件流控（uartx、波特率、输出端口、队列）
void uart_init_no_hwfc(uart_port_t uart_num, uint32_t baud_rate, int tx_io_num, int rx_io_num, int queue_size, QueueHandle_t *uart_queue);

// uart发送字符串
int uart_sendData(uart_port_t uart_num, const char* data);

// uart接收
int uart_read_bytes(uart_port_t uart_num, void* buf, uint32_t length, TickType_t ticks_to_wait);
```


## 注意事项

* uart可以映射到除仅能做输入 和 6、7、8、9、10、11之外的任意端口。
除了uart0，没有默认端口这一说。
