# 29_SPI_AS5047P

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **spi_as5047p_task** ，初始化 SPI3-`AS5047P`磁编码器，并连续读磁极转子角度。

使用 `spi_config.c.h` 驱动模块，对ESP32的 `SPI` 进行配置。

使用 `spi_as5047p.c.h` 驱动挂载在 `SPI` 总线上的 `AS5047P`磁编码器，并连续读磁极转子角度。

## AS5047P 磁编码器介绍

`AS5047P` 是一款 14 位的绝对角度位置传感器，其用于在整个 360 度范围内进行角度测量。有 SPI、ABI、PWM、UVW 等多种位置角度输出方式，适合在高速伺服电机领域替代传统光电编码器。

关于`AS5047P`的详细介绍，请见我的博客：

AS5047P磁编码器应用设计大全解：硬件电路设计、SPI通信时序、逻辑波形分析、注意事项：https://blog.csdn.net/Mark_md/article/details/119645201?spm=1001.2014.3001.5501

ODrive踩坑（四）AS5047P-SPI绝对值磁编码器：https://blog.csdn.net/Mark_md/article/details/119774663?spm=1001.2014.3001.5501


## 硬件连接

|                  | CLK    | MISO   | MOSI   | CS     |
| ---------------- | ------ | ------ | ------ | ------ |
| ESP32 SPI Master | GPIO18 | GPIO19 | GPIO23 | GPIO17 |
| AS5047P          | CLK    | MISO   | MOSI   | CS     |

CS0  17  // 软CS0引脚（硬件CS太快，易导致通信失败）

开发板扩展SPI设备，可将其插接到 J5位。需拔掉背部的LCD液晶模组排线。


## 运行现象

* LED闪烁。

* 终端连续输出 AS5047P `原始值`、`转子实际角度`。


## 关键函数

```c
// 配置SPIx主机模式，配置DMA通道、DMA字节大小，及 MISO、MOSI、CLK的引脚。
void spi_master_init(spi_host_device_t host_id, int dma_chan, uint32_t max_tran_size, gpio_num_t miso_io_num, gpio_num_t mosi_io_num, gpio_num_t clk_io_num);

// AS5047P初始化，初始化SPI总线，配置为 SPI mode 1.(CPOL=0, CPHA=1)，CS引脚使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
void spi_as5047p_init(spi_host_device_t host_id, uint32_t clk_speed, gpio_num_t cs_io_num);

// 读AS5047P角度的原始值（适用于连续读的情况，需要注意当前的数据是上一帧命令的返回值）
uint16_t as5047p_read_angle_continuous(spi_device_handle_t spi, gpio_num_t cs_io_num);

// 将AS5047P角度的原始值转换为实际360°角度（如角度返回错误，值会 >= 360°）
float as5047p_to_angle(uint16_t data);
```


## 注意事项

* `AS5047P` 的SPI时序为：SPI mode 1.(CPOL=0, CPHA=1)

* `AS5047P` 支持3.3~5V供电，与ESP32连接时请注意通信电平。

* `AS5047P`的驱动中，CS引脚为软件实现，硬CS会导致通信失败。

* SPI时序`逻辑分析仪`图、`流程顺序图`、`寄存器表结构`，请见上面博客的链接。

* 下一个Demo演示的是`TLE5012B`的SPI通信。与`AS5047P`一样，都是支持SPI通信的`磁编码器`。虽然`TLE5012B`的分辨率更高，但系统延时过大，总体性能`AS5047P`>`TLE5012B`。

`AS5047P`与`TLE5012B`参数对比：https://blog.csdn.net/Mark_md/article/details/119524819?spm=1001.2014.3001.5501

常见磁编码器参数对比：https://blog.csdn.net/Mark_md/article/details/100181701
