# 14_MCPWM_HALF_BRIDGE

## 例程简介

介绍ESP32的MCPWM的使用，初始化PWM输出，定时器同步相位，PWMA/PWMB反相互补输出。观察示波器的通道波形。half_bridge0输出反相且连续变化的PWM，half_bridge2相位连续发生变化。

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **mcpwm_half_bridge_task** ，初始化3路半桥驱动，全部反相输出，观察示波器的通道波形

使用 `mcpwm_half_bridge.c.h` 驱动模块，来对ESP32的 `MCPWM` 的半桥输出进行配置。

ESP32的`MCPWM`较为复杂，建议在大概了解`MCPWM`硬件组成后，再去使用easyIO的API。


## 运行现象

* `half_bridge1`、`half_bridge2`的 `PWMA/PWMB` 波形为反相互补输出。

* `half_bridge0`输出反相且连续变化的PWM，并驱动电机，驱动电机不会动但会产生啸叫。

* `half_bridge2`相位连续发生变化。


## 学习内容

1. ESP32使用`MCPWM`输出PWM，模仿控制半桥驱动。

2. 半桥PWM的`反相输出`、`相位同步`、`相位移位`、`死区控制`（ESP-IDF有Bug，反相和死区同时使用会导致两者均无效）。

3. easyIO最多支持6路半桥输出（共12个GPIO）。


## 关键函数

```c
// 半桥PWM驱动初始化及配置
void mcpwm_half_bridge_init(half_bridge_t half_bridge_num, uint16_t frequency, output_type_t output_type, int PWMxA_gpio_num, int PWMxB_gpio_num);

// 半桥PWM驱动相位同步配置
void mcpwm_half_bridge_sync(half_bridge_t half_bridge_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val, int sync_gpio_num);

// 半桥PWM占空比输出
void mcpwm_half_bridge_output(half_bridge_t half_bridge_num, float PWMxA_duty, float PWMxB_duty);

// 半桥相位调整
void mcpwm_half_bridge_change_phase(half_bridge_t half_bridge_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val);

// 设置某一通道PWM的占空比
esp_err_t mcpwm_set_duty(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, mcpwm_generator_t gen, float duty);
// 设置某一通道PWM的一个周期脉冲的高电平时间
esp_err_t mcpwm_set_duty_in_us(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, mcpwm_generator_t gen, uint32_t duty_in_us);
```


## 注意事项

* 半桥的输出引脚支持任意（除了只能做输入的那几个GPIO）GPIO映射。（内部有GPIO矩阵，十多M以下的低速数字信号都支持任意映射）

* `half_bridge`跟`dc_motor`的API基本是一样的，只是换了名字和枚举列表，套壳。

* 功能上比 `dc motor` 多了`PWMA/PWMB反相互补`。

* 死区部分没有调通。（可能也是乐鑫的Bug，单独使用死区是能看到死区的，但死区和反相互补同时使用，就会导致配置全部失效）（不过这个Bug问题不大，可以外接反相器解决，而且有的半桥驱动器就自带反相）

* easyIO默认`half_bridge`的输出GPIO与`dc_motor`是相同，而且都是使用的`MCPWM0/1`，使用时要防止硬件冲突。

* 同步的信号源 `MCPWM_SELECT_SYNC0`、`MCPWM_SELECT_SYNC1`、`MCPWM_SELECT_SYNC2`，暂未找到连接MCU内部信号的API；如使用PWM相位同步，则必须使用外部输入端口。