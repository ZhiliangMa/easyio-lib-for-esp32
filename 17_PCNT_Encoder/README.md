# 17_PCNT_Encoder

## 例程简介

介绍ESP32的 `Pulse_cnt` 的使用，进行`ABI正交编码器`、`Step/Direction步进/方向 编码器`的计数。并通过时间系数、距离系数，获取`累计计数`、`速度`、`距离`。

使用 `FreeRTOS` 的 `Task`，创建三个任务：

1. **led_task** ，控制LED闪烁

2. **pcnt_encoder_count_task** ，初始化PCNT_UNIT_0，进行`ABI正交编码器`、`Step/Direction步进/方向 编码器`的计数

3. **encoder_simulation_task** ，用`mcpwm`来模拟编码器的+-90°正交编码输出，100Hz。

使用 `pulse_cnt.c.h` 驱动模块，来对ESP32的 `Pulse_cnt` 进行配置。进行`ABI正交编码器`、`Step/Direction步进/方向 编码器`的计数。


## 运行现象

* `GPIO_HALF_BRIDGE0_PWMA_OUT`、`GPIO_HALF_BRIDGE1_PWMA_OUT` 输出相位相差+-90°正交编码输出，来模拟`ABI编码器`的输出波形。
 
* 连接 `GPIO_HALF_BRIDGE0_PWMA_OUT` 到 `ENCODER0_INPUT_A`，连接 `GPIO_HALF_BRIDGE0_PWMB_OUT` 到 `ENCODER0_INPUT_B`，运行例程，观察串口调试输出。

* 随着 正交编码 方向的变化，pcnt计数也发生着变化，直接调用 `cal_pcnt_count_speed_dis`便可从`pcnt_count[PCNT_UNIT_MAX]`得到 `累计计数`、`速度`、`距离`。用户不用担心PCNT溢出，easyIO已在溢出中断中进行过溢出处理。


## 防止PCNT计数溢出思路

* `PCNT_H_LIM_VAL`为单计数器周期最大溢出计数。为了不频繁进入中断，此值应在int16_t的范围内设置的尽可能大。`easyio`默认设置为30000，无特殊需求不需要更改。

* 累计计数的思路：只使能最大/最小值溢出中断，进入中断后，对溢出次数计数++/--。于是 累计计数值 = 溢出次数 * PCNT_H_LIM_VAL + 当前PCNT的计数值


## 学习内容

1. 了解编码器分类、应用场合、输出方式，了解`ABI正交编码` 和 `步进/方向` 编码输出方式。

2. ESP32配置为 `正交编码1倍频`、`4倍频`的方法，和配置为 `步进/方向` 的方法。

3. ESP32的`PCNT`如何应用在编码器计数。


## 关键函数

```c
// ABZ正交编码器计数模式，1个旋转周期产生1个计数
void pcnt_encoder_ABZ_1_period_1_cnt_init(pcnt_unit_t unit, uint16_t filter_val, int Encoder_IO_A, int Encoder_IO_B);

// ABZ正交编码器计数模式，1个旋转周期产生4个计数，位置定位更加精准
void pcnt_encoder_ABZ_1_period_4_cnt_init(pcnt_unit_t unit, uint16_t filter_val, int Encoder_IO_A, int Encoder_IO_B);

// Step/Direction 步进脉冲/方向编码器计数模式，1个旋转周期产生2个计数
void pcnt_encoder_Step_Direction_1_period_2_cnt_init(pcnt_unit_t unit, uint16_t filter_val, int Encoder_IO_STEP, int Encoder_IO_DIR);

// 计算编码器输入的 累计计数、速度、距离
void cal_pcnt_count_speed_dis(pcnt_unit_t unit, float time, float sub);
```


## 注意事项

* `easyIO` 默认开启 最大/最小值 溢出中断，并已在中断中封装了对累计计数的处理，用户不用担心计数溢出问题。

* 用户在 初始化编码器模式后，直接调用此`cal_pcnt_count_speed_dis`API即可获得 编码器的 累计计数、速度、距离。

* 过零点中断、中间阈值中断在编码器计数的应用中不需要，不要使能。

* 短于`信号滤波值`的脉冲将被忽略，一般设置为100，1250ns=1.25us，足以允许 1000000/1.25/2=400KHz 的信号输入。

* `cal_pcnt_count_speed_dis`输入的时间系数，要与该函数的调用周期匹配，才能计算出精准的 speed速度

* `cal_pcnt_count_speed_dis`输入的距离系数，要与齿轮减速比、轮胎周长匹配，才能计算出精准的 dis距离
