# 27x1_I2C_SHT30

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建2个任务：

1. **led_task** ，控制LED闪烁

2. **i2c_sht30_task** ，初始化 SHT30工作于周期测量模式，获取环境温湿度数据


使用 `i2c_config.c.h` 驱动模块，对ESP32的 `I2C` 进行配置。

并用 `i2c_sht30.c.h` 驱动挂载在 `I2C` 总线上的`SHT30` 温湿度传感器。

`SHT30`继承于SHT2x系列温湿度传感器，进一步提高了产品可靠性和精度规格。传感器的湿度测量范围是0-100%RH，温度测量范围是-40-125℃。小米的智能家居设备很多都在使用这个系列的型号。

相比较于国产的AHT2x系列，`SHT30`参数上要好一些。而且手册也比较友好，使用起来没那么多顾虑。最重要的是`AHT20`与开发板的电容触摸屏-`FT6236U`的I2C从机地址相同，导致不能同时存在于一条I2C总线上。

`SHT30`的2字节写命令，可以看作向一个寄存器地址为8位的寄存器，写1个字节。

`SHT30`的读时序看似非标，但寻找 `SHT30读取一次数据（周期测量模式下）的时序图`的规律后发现 ，他的读时序是寄存器地址为16位的I2C读。与MPU6050等设备相比，
仅在寄存器地址段上多了一个字节+ACK。这里使用 i2c_master_read_slave_reg_16bit(i2c_port_t i2c_num, uint8_t slave_addr, uint16_t reg_addr, uint8_t *data_rd, size_t size, TickType_t ticks_to_wait);

`SHT30`驱动源码参考，特此感谢：https://cloud.tencent.com/developer/article/1662638


## 硬件连接

|                  | SCL    | SDA    |
| ---------------- | ------ | ------ |
| ESP32 I2C Master | GPIO14 | GPIO4 |
| SHT30            | SCL    | SDA    |


## 运行现象

* LED闪烁。

* 初始化 SHT30工作于周期测量模式，获取环境温湿度数据。


## 学习内容

1. I2C 总线时序。

2. 温湿度传感器 - SHT30 的使用操作。


## 关键函数

```c
// 配置I2Cx-主机模式，（I2C端口、总线速率、SCL引脚，SDA引脚）
esp_err_t i2c_master_init(i2c_port_t i2c_num, uint32_t clk_speed, gpio_num_t scl_io_num, gpio_num_t sda_io_num);

// 复位SHT30
void SHT30_reset(void);

// 初始化SHT30(周期测量模式)
uint8_t SHT30_Init(void);

// 从SHT30读取一次数据
uint8_t SHT30_Read_Dat(uint8_t* dat);

// 将SHT30接收的6个字节数据进行CRC校验，并转换为温度值和湿度值
uint8_t SHT30_Dat_To_Float(uint8_t* const dat, float* temperature, float* humidity);
```


## 注意事项

* `AHT20`与开发板的电容触摸屏-`FT6236U`的I2C从机地址相同，导致不能同时存在于一条I2C总线上。如果确定使用`FTxxxx`电容触摸屏，那么同I2C总线上的温湿度传感器建议选择`SHT30`或其他型号。

* 温湿度传感器 在硬件设计时，尽量将传感器周围的PCB镂空，以减小热传导对测量数据的影响。

* 如对传感器有抗污染需求，`SHT30`可搭配使用`SF2防水防尘罩`，透气，可过滤99.99%的大于0.1μm的颗粒。
