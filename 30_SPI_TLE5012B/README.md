# 30_SPI_TLE5012B

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **spi_tle5012b_task** ，初始化 SPI3-`TLE5012B`磁编码器，并连续读磁极转子角度。

使用 `spi_config.c.h` 驱动模块，对ESP32的 `SPI` 进行配置。

使用 `spi_tle5012b.c.h` 驱动挂载在 `SPI` 总线上的 `TLE5012B`磁编码器，并连续读磁极转子角度。

## TLE5012B 磁编码器介绍

`TLE5012B` 是一款 15 位的绝对角度位置传感器，其用于在整个 360 度范围内进行角度测量。有 SSC、PWM、增量接口 (IIF)、霍尔开关模式 (HSM)、短 PWM 等多种对外接口，适合替代传统光电编码器。

关于`TLE5012B`的详细介绍，请见我的博客：

TLE5012B 硬件电路设计、4线SPI通信，驱动完美兼容4线SPI不用改MOSI开漏推挽输出：https://blog.csdn.net/Mark_md/article/details/119806139?spm=1001.2014.3001.5501

ODrive踩坑（二）3508电机和TLE5012B磁编码器参数配置：https://blog.csdn.net/Mark_md/article/details/117898801?spm=1001.2014.3001.5501


## 硬件连接

|                  | CLK    | MISO   | MOSI   | CS     |
| ---------------- | ------ | ------ | ------ | ------ |
| ESP32 SPI Master | GPIO18 | GPIO19 | GPIO23 | GPIO17 |
| TLE5012B         | CLK    | MISO   | MOSI   | CS     |

CS0  17  // 软件的CS0引脚（硬CS太快，易导致通信失败）


## 运行现象

* LED闪烁。

* 终端连续输出 TLE5012B `原始值`、`转子实际角度`。


## 关键函数

```c
// 配置SPIx主机模式，配置DMA通道、DMA字节大小，及 MISO、MOSI、CLK的引脚。
void spi_master_init(spi_host_device_t host_id, int dma_chan, uint32_t max_tran_size, gpio_num_t miso_io_num, gpio_num_t mosi_io_num, gpio_num_t clk_io_num);

// TLE5012B初始化，初始化SPI总线，配置为 SPI mode 1.(CPOL=0, CPHA=1)，CS引脚使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
void spi_tle5012b_init(spi_host_device_t host_id, uint32_t clk_speed, gpio_num_t cs_io_num);

// 读TLE5012B角度的原始值（适用于只读一次的情况）
uint16_t tle5012b_read_angle(spi_device_handle_t spi, gpio_num_t cs_io_num);

// 将TLE5012B角度的原始值转换为实际360°角度
float tle5012_to_angle(uint16_t data);

// 读TLE5012B速度的原始值（适用于只读一次的情况）
uint16_t tle5012b_read_speed(spi_device_handle_t spi, gpio_num_t cs_io_num);
```


## 注意事项

* `TLE5012B` 的SPI时序为：SPI mode 1.(CPOL=0, CPHA=1)

* `TLE5012B` 支持3.3~5V供电，与ESP32连接时请注意通信电平。

* `TLE5012B`的驱动中，CS引脚为软件实现，硬CS会导致通信失败。

* SPI时序`逻辑分析仪`图、`流程顺序图`、`寄存器表结构`，请见上面博客的链接。

* `TLE5012B`与`AS5047P`一样，都是支持SPI通信的`磁编码器`。虽然`TLE5012B`的分辨率更高，但系统延时过大，总体性能`AS5047P`>`TLE5012B`。

`AS5047P`与`TLE5012B`参数对比：https://blog.csdn.net/Mark_md/article/details/119524819?spm=1001.2014.3001.5501

常见磁编码器参数对比：https://blog.csdn.net/Mark_md/article/details/100181701
