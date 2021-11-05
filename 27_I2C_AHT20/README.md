# 27_I2C_AHT20

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **i2c_aht20_task** ，初始化AHT20温湿度传感器，读取温湿度

使用 `i2c_config.c.h` 驱动模块，对ESP32的 `I2C` 进行配置。

并用 `i2c_aht20.c.h` 驱动挂载在 `I2C` 总线上的`AHT20` / `AHT21`温湿度传感器。


## 硬件连接

|                  | SCL    | SDA    |
| ---------------- | ------ | ------ |
| ESP32 I2C Master | GPIO14 | GPIO4 |
| AHT20            | SCL    | SDA    |

`ESP32-IOT-KIT`没有板载AHT20，选而使用了性能更好的`SHT30`，见下一个Demo。

## 运行现象

* LED闪烁。

* 初始化AHT20温湿度传感器，读取温湿度，并在终端中显示。

* 向传感器吹气，观察温湿度示数。


## 学习内容

1. I2C 总线时序。

2. 温湿度传感器功能及应用场景。


## 关键函数

```c
// 配置I2Cx-主机模式，（I2C端口、总线速率、SCL引脚，SDA引脚）
esp_err_t i2c_master_init(i2c_port_t i2c_num, uint32_t clk_speed, gpio_num_t scl_io_num, gpio_num_t sda_io_num);

// AHT20 设备初始化
uint8_t i2c_aht20_init(i2c_port_t i2c_num);

// AHT20 设备读取 相对湿度和温度（原始数据20Bit）
uint8_t i2c_aht20_read_ht(i2c_port_t i2c_num, uint32_t *HT);

// AHT20 温湿度信号转换（由20Bit原始数据，转换为标准单位RH=%，T=°C）
uint8_t aht20_standard_unit_conversion(struct m_AHT20* aht);
```


## 注意事项

* AHT20的I2C器件地址为0x38，与配套电容触摸屏的FT5206、FT5336、FT5436、FT6206、FT6236的I2C器件地址冲突。都是0x38。建议更换传感器型号，如SHT30。

* `ESP32-IOT-KIT`没有板载AHT20，选而使用了性能更好的`SHT30`，见下一个Demo。

* 编译此代码不要用 `IDF-V4.3`，建议使用 `IDF-V4.2`。easyio库用IDF-V4.3编译I2C，运行后调试会出现许多莫名其妙的Bug，请等待乐鑫官方更新。
