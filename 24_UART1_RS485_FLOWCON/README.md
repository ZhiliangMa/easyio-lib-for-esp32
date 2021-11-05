# 24_UART1_RS485_FLOWCON

## 例程简介

ESP32的 `UART` 硬件，用`硬件流控`做`RS485`通信。

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **uart_rs485_echo_task** ，配置uart1-txrx，设置`硬件流控`以用于`RS485`通信。发送并接收数据。

外接`RS485`总线设备，观察调试结果。

使用 `uart_config.c.h` 驱动模块，来对ESP32的 `UART` 进行配置。进行UART1-`RS485`总线通信操作。

IDF的Demo例表中有RS485的FreeModbus，有兴趣的可以运行看下。
运行GPS的NEMA解析时报错，暂无GPS例程，待以后完善填坑。


# 硬件连接

开发板的`RS485`与`CAN`芯片共用同2个GPIO，使用RS485需要将`J8`位的两个跳线帽跳接到上面。


## 运行现象

* LED闪烁。

* 将`ESP32-IOT-KIT`开发板的J8，用跳线帽将 IO13、IO15 短接到`RS485`的位置。

* 将开发板的CN2-RS485端口，连接`USB转RS485`设备，发送消息，观察双发的收发。

* `USB转RS485`设备发送消息，ESP32接收到消息后会原路返回相同内容的消息。


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

* ESP32做`RS485`通信，使用`硬件流控`，需要将`MAX3485`的Pin2、3（RE、DE）短接，连接到ESP32的硬件流控。

* 因为RS485为半双工，总线上某一设备长时间占用总线发送消息，会导致其他设备不能正常通信。

* `ESP-IDF`的Demo中，有 RS485 的 `Free ModBUS `例程。位置在 protocols -> modbus -> serial -> mb_master/mb_slave。

* 本来`ESP-IDF`的Demo中，还有GPS的例程-`nmea0183_parser`。测试时没有运行成功，不知道为什么接收不到GPS数据，待以后有空补全一起合并到工程库中。
