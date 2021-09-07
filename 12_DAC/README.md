# 12_DAC

## 例程简介

介绍ESP32的`DAC`的使用，初始化`DAC channel1`，生成`三角波`，并用ADC测量端口电压值打印输出。

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **dac_output_task** ，初始化`DAC_CHANNEL_1`；并生成三角波，并用`ADC1-CH6`测量输出电压

使用 `dac_output.c.h` 驱动模块，来对ESP32的 `DAC` 进行配置。

ESP32的DAC较为简单，常用的也就2个API，不可更改输出GPIO，仅对原有ESP-IDF的API进行了简单的封装。


## 运行现象

* `DAC channel1`(GPIO25)生成`三角波`，ADC-CH6与DAC直连测量其输出电压值。

* 串口打印输出 DAC输出电压、ADC输入电压。

* 使用示波器能观察到`三角波波形`。


## 学习内容

1. DAC的使用。

2. easyIO最多支持2路DAC的输出，且不支持任意GPIO映射，引脚映射表在`dac_output.c.h`文件中。


## 关键函数

```c
// DAC输出通道x初始化
void dac_channel_init(dac_channel_t channel);

// DAC通道x输出电压xxmV（供电电源为3.3V时）
void dac_output_0_3V3_voltage_int_mV(dac_channel_t channel, int voltage);

// DAC通道x输出电压xxV（供电电源为3.3V时）
void dac_output_0_3V3_voltage_float_V(dac_channel_t channel, float voltage);
```


## 注意事项

- easyIO最多支持2路DAC的输出，且不支持任意GPIO映射，引脚映射表在`dac_output.c.h`文件中。

- DAC端口的输出电流很微弱，一般只有几mA，且在电流较大时有较大误差，不能直接带大电流负载。如需对外做信号输出，建议在硬件电路中增加跟随器。