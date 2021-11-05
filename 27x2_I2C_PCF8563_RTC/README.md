# 27x2_I2C_PCF8563_RTC

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建4个任务：

1. **led_task** ，控制LED闪烁

2. **key_scan_task** ，单独运行一个任务，运行按键检测（滤除按键抖动、短按、长按）

3. **key_catch_task** ，去捕获按键事件，来手动重置PCF8563的RTC日期、时间。

4. **i2c_pcf8563_rtc_task** ，初始化 使用I2C总线的RTC芯片-PCF8563，来进行离线低功耗计时

使用 `i2c_config.c.h` 驱动模块，对ESP32的 `I2C` 进行配置。

并用 `i2c-pcf8563.c.h` 驱动挂载在 `I2C` 总线上的`PCF8563` RTC芯片。

绝大多数的`ESP32`模组，硬件设计时直接将`VDD3P3_RTC`引脚接到了`VDD33`电源上，使其外接纽扣电池变得异常困难。另外因ESP32的IO数目较少，很难为了外接`32.768K晶振`而牺牲两个IO口，多会使用内部低速时钟，从而导致计数误差比较大。以上种种原因之下，`ESP32模组`的设计很难发挥出自身RTC应有的功效。故低功耗计时的应用，建议外挂一片RTC芯片，以避免不必要的麻烦。

`PCF8563` 是一款 `实时时钟(Real Time Clock)芯片`，它通过I2C总线与主机进行通信，具有极低的运行功耗，运行电流的典型值为 0.25μA。
具有万年历功能，可获得 世纪、年、月、日、星期、时、分、秒 等日期/时间消息。支持闹钟和计时功能，并提供1中断输出接口和1可编程时钟输出。【与其他的常用RTC芯片相比，如DS1307，DS1307的工作电压为5V，不支持3.3V电平】

在`ESP32-IOT-KIT`开发板上，其I2C接口(IO14、IO4)连接了一片`PCF8563`。`CR1220电池`和3V3电源为其供电，两者通过肖特基二极管来自动切换，以保证`PCF8563`长时间不掉电计时。

【注意】：开发板板载的`CR1220电池`为一次性电池，切不可摘下充电，以免发生事故。

`PCF8563`驱动代码移植参照，特此感谢：https://blog.csdn.net/weixin_41756255/article/details/84996464


## 硬件连接

|                  | SCL    | SDA    |
| ---------------- | ------ | ------ |
| ESP32 I2C Master | GPIO14 | GPIO4  |
| PCF8563          | SCL    | SDA    |


## 运行现象

* LED闪烁。

* 初始化 使用I2C总线的RTC芯片-PCF8563，来进行离线低功耗计时。

* 预设的初始日期、时间为 2021年10月1日 星期5 12:00。程序运行后，RTC开始运行并计时，在终端中输出当前日期、时间。

* 短按 `BOOT` 按键，计时会被重置回 初始值。

* 长按 `BOOT` 按键，计时将被设置为 2021年10月1日 星期5 23:59，等待1分钟可观察日期的切换效果。


## 学习内容

1. I2C 总线时序。

2. RTC芯片 - PCF8563 的使用操作。


## 关键函数

```c
// 配置I2Cx-主机模式，（I2C端口、总线速率、SCL引脚，SDA引脚）
esp_err_t i2c_master_init(i2c_port_t i2c_num, uint32_t clk_speed, gpio_num_t scl_io_num, gpio_num_t sda_io_num);

// 检测 PCF8563（总线上是否存在 PCF8563 器件？芯片是否是坏片？）
uint8_t PCF8563_Check(void);

// 启动 PCF8563
void PCF8563_Start(void);

// 设置 PCF8563 时间，主要用于后台调用，或者初始化时间用（年、月、日、星期、时、分）
void PCF8563_Set_Times(uint8_t PCF_Format,\
                       uint8_t PCF_Century,\
	                   uint8_t Year, uint8_t Month, uint8_t Date, uint8_t Week,\
                       uint8_t Hour, uint8_t Minute);

// 设置 PCF8563 时间信息。（时、分、秒）
void PCF8563_SetTime(uint8_t PCF_Format, _PCF8563_Time_Typedef* PCF_DataStruct);

// 读取 PCF8563 时间信息
void PCF8563_GetTime(uint8_t PCF_Format, _PCF8563_Time_Typedef* PCF_DataStruct);

// 读取 PCF8563 日期信息
void PCF8563_GetDate(uint8_t PCF_Format, uint8_t *PCF_Century, _PCF8563_Date_Typedef* PCF_DataStruct);
```


## 注意事项

* 在初始化PCF8563时，只需调用 PCF8563_Start(); 此api会自动根据IC运行模式，将 PCF8563 启动计数。而不用再使用 PCF8563_SetMode();

* 如发现 PCF8563 计时存在误差时，排除掉常识性的硬件故障，可微调 32.768K晶振两端的电容进行校准调节。

* `PCF8563`与其他的常用RTC芯片相比，如DS1307，DS1307的工作电压为5V，不支持3.3V电平。
