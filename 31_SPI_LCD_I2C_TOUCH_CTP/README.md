# 31_SPI_LCD_I2C_TOUCH_CTP

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **lcd_ctp_task** ，初始化 SPI3-LCD，和电容触摸屏 FTxxxx。并在屏幕上显示触摸轨迹及信息。

使用 `i2c_ctp_ftxx06_xx36.h` 驱动挂载在 `I2C` 总线上的 `FT5206、FT5336、FT5436、FT6206、FT6236`电容触摸屏，获取触摸点数、坐标、触摸事件、事件ID。

`FT5206` 为单点触摸控制器。`FT5336` 为`5`点触摸控制器。

例程可使用 `FT5206` 和 `FT5336`任意一款，Demo最多支持`5`点轨迹。


## 硬件连接

|                  | CLK    | MOSI   | DC     | BLK    | RST    | CS     |
| ---------------- | ------ | ------ | ------ | ------ | ------ | ------ |
| ESP32 SPI Master | GPIO18 | GPIO23 | GPIO22 | GPIO21 | EN     | GPIO5  |
| lcd              | CLK    | MOSI   | DC     | BLK    | RST    | CS     |

<br/>

|                  | SCL    | SDA    |
| ---------------- | ------ | ------ |
| ESP32 I2C Master | GPIO16 | GPIO17 |
| FT5336           | SCL    | SDA    |


## 运行现象

* LED闪烁。

* 液晶屏上显示触摸点数、坐标值、触摸事件、事件ID，当有手指滑动时，会在屏幕上以不同颜色留下点轨迹，Demo最多支持`5`点轨迹。


## 关键函数

```c
// 配置I2Cx-主机模式，（I2C端口、总线速率、SCL引脚，SDA引脚）
esp_err_t i2c_master_init(i2c_port_t i2c_num, uint32_t clk_speed, gpio_num_t scl_io_num, gpio_num_t sda_io_num);

// FTxxxx触控芯片初始化。（将 DEVIDE_MODE 写0，使IC处于工作模式即可。其他参数默认。）
void i2c_ctp_FTxxxx_init(i2c_port_t i2c_num);

// 读FTxxxx 所有触摸信息。（触摸点个数、坐标、事件、ID）
void i2c_ctp_FTxxxx_read_all(i2c_port_t i2c_num, ctp_tp_t* ctp);

/**
 * @brief  触摸IC的信息结构体，方便管理触摸发生的5个点信息
 */
typedef struct {
    uint8_t event; // 事件类型：0-按下；1-抬起；2-接触/长按；3-无事件
    uint8_t id; // 触摸ID
    uint16_t x; // 触摸产生的点的X坐标
    uint16_t y; // 触摸产生的点的Y坐标
} ctp_tp_info_t;

typedef struct {
    uint8_t tp_num; // 发生触摸的点数量
    ctp_tp_info_t tp[5];
} ctp_tp_t;
```


## 注意事项

* FT5206、FT5336、FT5436、FT6206、FT6236，驱动几乎都是一样（区别是触摸点数、刷新率不同）

* 编译此代码不要用 `IDF-V4.3`，建议使用 `IDF-V4.2`。IDF-V4.3在编译I2C，运行后调试会出现许多莫名其妙的Bug，请等待乐鑫官方更新。

* 在`IDF-V4.3`中编译、下载运行会报错：
```
i2c_param_config(645): i2c clock choice is invalid, please check flag and frequency
i2c_set_pin(826): scl and sda gpio numbers are the same
```
而在 `IDF-V4.2` 中，是可以被正常编译、下载和运行的。

* 运行此Demo时，I2C总线上不要有AHT20设备。AHT20的I2C器件地址为0x38，与电容触摸屏的FT5206、FT5336、FT5436、FT6206、FT6236的I2C器件地址冲突。都是0x38。

* 如准备使用I2C总线的温湿度传感器，不要选择AHT20。1.与FTxxxx触摸屏的I2C器件地址冲突；2.精度不是很好，测量出的温度偏高。
