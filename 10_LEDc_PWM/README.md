# 10_LEDc_PWM

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **ledc_pwm_hs_rgb3ch_task** ，使用高速ledc通道，控制RGB-LED呼吸渐变

2. **ledc_pwm_ls_led1ch_task** ，使用低速ledc通道，控制单色LED以固定占空比输出

使用 ledc_pwm.c.h 驱动模块，来对ESP32的 `led control` 进行配置

鉴于数据手册对 `led control` 的说明很少，不建议了解具体硬件细节，直接使用`easyio`封装好的函数去调用。

【注意】：因开发板仅板载1个用户LED，此例程纵使可以演示众多GPIO的输出，但在开发板上仅能显示1个。

## 硬件连接

|        | LED    |
| ------ | ------ |
| ESP32  | GPIO33 |

例程中使用了3通道高速LEDc、1通道低速LEDc，如需修改通道数目、GPIO请修改 `ledc_pwm.h`。

## 运行现象

* LED 呼吸渐变，并有串口信息输出。

* 单色LED 分别输出0%、50%、100%占空比，并有串口信息输出。

* 观察示波器，以上IO口的PWM频率均为`5KHz`。但由于没有使用同一定时器，所以跳变沿时序不一致。


## 学习内容

1. 鉴于数据手册对 `led control` 的说明很少，不建议了解具体硬件细节，直接使用`easyio`封装好的函数去调用。


## 关键函数

```c
// 初始化高速ledc通道，5KHz，13Bit分辨率，共使用3个通道，对应RGB-LED的管脚
void ledc_pwm_hs_init(void);

// 初始化低速ledc通道，5KHz，13Bit分辨率，共使用1个通道，对应单色LED的管脚
void ledc_pwm_ls_init(void);

// 指定高速/低速ledc通道，在设定的时间内，从当前占空比 渐变到 期望占空比（0~1000‰，省去了手动计算，更加方便）
void ledc_pwm_set_fade_duty_cycle(ledc_channel_config_t ledc_hs[], uint32_t ch, uint32_t duty_cycle, uint32_t time);

// 指定高速/低速ledc通道，设置固定占空比输出（0~1000‰，省去了手动计算，更加方便）
void ledc_pwm_set_duty_cycle(ledc_channel_config_t ledc_hs[], uint32_t ch, uint32_t duty_cycle);
```


## 注意事项

- 鉴于数据手册对 `led control` 的说明很少，不建议了解具体硬件细节，直接使用`easyio`封装好的函数去调用

- 以定时器的分辨率去输出脉宽时（`ledc_pwm_set_fade / ledc_pwm_set_duty`），要根据使用的定时器的分辨率手动计算。本例程使用的定时器分辨率为13Bit，输出脉宽范围0~100%（对应0~8191）。

- 由于ledc的参数较多，频率参数不建议随意更改，如需更改请移步`ledc_pwm.h`的宏定义，请遵照手册的参数去配置

- ESP32的ledc分为高速、低速通道，各有8个输出通道，共16通道。

- 硬件板载LED数量受限，仅使用GPIO-18演示LED渐变。其他通道也均可用，可用示波器看波形，但碍于无外接的LED，无法直观显示。
