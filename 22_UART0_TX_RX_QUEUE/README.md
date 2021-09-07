# 22_UART0_TX_RX_QUEUE

## 例程简介

ESP32的 `UART` 硬件，用于UART串口通信。

使用 `FreeRTOS` 的 `Task`，创建三个任务：

1. **led_task** ，控制LED闪烁

2. **uart_tx_task** ，配置uart0-txrx，使用队列接收串口消息。

3. **uart_rx_task** ，uart0-rx接收数据，使用队列方式去接收数据。

因为使用默认的`UART0`端口，所以并不需要额外的硬件连接。（开发板的`UART0`与`CH340K`连接，可从终端或者串口助手上查看）

本`Demo`建议使用`串口调试助手`，因为使用终端需要调整为中文输入法，用英文输入法发送每次几乎只能发送一两个字节，影响演示效果。

使用 `uart_config.c.h` 驱动模块，来对ESP32的 `UART` 进行配置。进行UART0串口通信操作。


## 运行现象

* LED闪烁。

* 运行串口调试助手，波特率115200，打开开发板端口。发送普通的消息，如"uart, Hello!"，观察提示信息。


## 学习内容

1. ESP32的`UART`配置及用法。

2. 使用`队列`的方式，接收UART的数据。

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

// uart事件结构体
typedef struct {
    uart_event_type_t type; /*!< UART event type */
    size_t size;            /*!< UART data size for UART_DATA event*/
    bool timeout_flag;      /*!< UART data read timeout flag for UART_DATA event (no new data received during configured RX TOUT)*/
                            /*!< If the event is caused by FIFO-full interrupt, then there will be no event with the timeout flag before the next byte coming.*/
} uart_event_t;
```


## 注意事项

* 关于ESP32的UART，还有个很有意思的功能 - `模式检测`，见下个Demo的介绍。
