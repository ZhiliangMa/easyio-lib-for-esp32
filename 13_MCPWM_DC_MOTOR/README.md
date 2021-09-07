# 13_MCPWM_DC_MOTOR

## 例程简介

介绍ESP32的 `MCPWM` 的使用，初始化`PWM`输出，定时器间同步相位，输出PWM控制信号驱动直流有刷电机正转反转。

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **mcpwm_dc_motor_task** ，初始化3路电机PWM信号，其中一路驱动硬件H桥芯片DRVxxxx，使电机正转反转。

使用 `mcpwm_motor.c.h` 驱动模块，来对ESP32的 `MCPWM` 进行配置。

ESP32的`MCPWM`较为复杂，建议在大概了解`MCPWM`硬件组成后，再去使用easyIO的API。


## 运行现象

- 电机正转2s、反转2s、停止2s，接着正转2s...反复。

- 将`GPIO_MC0_SYNC0_IN`，使用跳线帽连接到一路PWM输出，使`MCPWM0`的全部PWM信号同步。

- `示波器通道1`输入`GPIO_MOTOR0_PWMA_OUT`的信号，将`示波器通道1`设为触发源。

- 用`示波器通道2`分别测试 `GPIO_MOTOR0_PWMA_OUT`、`GPIO_MOTOR1_PWMA_OUT`、`GPIO_MOTOR2_PWMA_OUT`，观察是否为同相？(前两个同相，最后一个相位左移20%)

* 之后拔除`GPIO_MC0_SYNC0_IN`跳线帽，将同步信号`SYNC0`空置。点动`复位`，再去观察各信号是否同相？（不知道这是不是乐鑫的Bug，如果同步信号`SYNC0`空置，上电第一次三个定时器依然会保持同步，但复位后就不同步了。按理来说同步信号源无信号就不会同步了，所以如果需要PWM同步，同步信号引脚必须连接信号！！）


## 学习内容

1. ESP32使用`MCPWM`输出PWM，控制有刷直流电机。

2. H桥电机驱动的硬件组成，及输入控制信号的波形。

3. easyIO最多支持6路直流有刷电机的驱动（2组`MCPWM`，每组3对PWM-A/B。共12个GPIO。根据直流电机划分为`motor0 ~ motor5`共6组直流电机）。


## 关键函数

```c
// 直流有刷电机PWM驱动初始化及配置
void mcpwm_dc_motor_init(dc_motor_t motor_num, uint16_t frequency, int PWMxA_gpio_num, int PWMxB_gpio_num);

// 直流有刷电机PWM相位同步配置
void mcpwm_dc_motor_sync(dc_motor_t motor_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val, int sync_gpio_num);

// 电机正转，占空比 = duty_cycle %
void dc_motor_forward(dc_motor_t motor_num, float duty_cycle);

// 电机反转，占空比 = duty_cycle %
void dc_motor_backward(dc_motor_t motor_num, float duty_cycle);

// 电机停止
void dc_motor_stop(dc_motor_t motor_num);

// 设置某一通道PWM的占空比
esp_err_t mcpwm_set_duty(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, mcpwm_generator_t gen, float duty);
// 设置某一通道PWM的一个周期脉冲的高电平时间
esp_err_t mcpwm_set_duty_in_us(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, mcpwm_generator_t gen, uint32_t duty_in_us);
```


## 注意事项

- `MCPWM`的信号可映射到任意GPIO输出管脚。ESP32：0-19, 21-23, 25-27, 32-39。GPIO 34-39 仅用作输入管脚。（内部有GPIO矩阵，十多M以下的低速数字信号都支持任意映射）

- `easyIO`默认使用 `MCPWM0` 驱动`motor0~2`, `MCPWM1` 驱动`motor3~5`，如有其他功能占用冲突，请修改`"easyio_mcpwm_config.h"`中宏定义。

- 开发板的电源要充足且稳定，直流电机的启动和停止会对电源有一定冲击。驱动小电机时建议使用充电器，而不是电脑USB口。（主板设计时会在USB添加过流保护IC）

- 同步信号的GPIO(`GPIO_MC0_SYNC0_IN`)要连接外部信号输入，不能空置，否则复位后会导致不能同步。（不知道这是不是乐鑫的Bug，如果同步信号`SYNC0`空置，上电第一次三个定时器还会保持同步，但复位后就不同步了。所以如果需要PWM同步，同步信号引脚必须连接信号！！）

- 如果要各PWM同相，同步信号连接有PWM输出的任意引脚均可。

- `easyIO`提供的`API`允许 `motor0~5` 各自使用不同的`频率`、`相位`。（得益于ESP32硬件设计的不错，每个`MCPWM`包含3个`Timer`，每个`Timer`都可独立运行，3个操作器再从3个`Timer`间选择信号，能根据应用场景灵活配置。而stm32的定时器做相位调整就比较麻烦）

- 同步的信号源 `MCPWM_SELECT_SYNC0`、`MCPWM_SELECT_SYNC1`、`MCPWM_SELECT_SYNC2`，暂未找到连接MCU内部信号的API；如使用PWM相位同步，则必须使用外部输入端口。