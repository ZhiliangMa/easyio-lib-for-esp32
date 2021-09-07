# 15_MCPWM_SERVO

## 例程简介

介绍ESP32的`MCPWM`的使用，初始化`PWM`输出，定时器同步相位。观察示波器的通道波形。舵机会来回摆动。

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **mcpwm_servo_task** ，初始化6路舵机，`servo0`来回摆动，`servo1~5`的摆角各不相同

使用 `mcpwm_servo.c.h` 驱动模块，来对ESP32的 `MCPWM` 进行配置，进而驱动舵机舵臂摆角。


## 运行现象

* `servo0`来回摆动，`servo1~5`的摆角各不相同。

* 观察示波器各通道波形。


## 学习内容

1. 普通舵机的控制信号波形、控制方式。

2. ESP32使用`MCPWM`输出50Hz的PWM，控制普通舵机摆动。

3. `easyIO`最多可以用`MCPWM`控制多达12路舵机，如需更多，可以使用`LEDC`输出更多通道的`50Hz-PWM`。


## 关键函数

```c
// Servo舵机配置
void mcpwm_servo_init(servo_unit_t servo_unit_num, uint16_t frequency, int PWMxA_gpio_num, int PWMxB_gpio_num);

// 舵机PWM相位同步配置
void mcpwm_servo_sync(servo_unit_t servo_unit_num, mcpwm_sync_signal_t sync_sig, uint32_t phase_val, int sync_gpio_num);

// 控制舵机的角度
void servo_degree(servo_t channel, float angle);
```


## 注意事项

* 舵机的输出引脚支持任意（除了只能做输入的那几个GPIO）GPIO映射。（内部有GPIO矩阵，十多M以下的低速数字信号都支持）

* easyIO默认`half_bridge`、`dc_motor`、`servo`的IO是相同，而且都是使用的`MCPWM0/1`，使用时要防止冲突。

* 舵机API中的频率可设置，以兼容`PWM>50Hz`的数字舵机

* 因为`ESP32`的`MCPWM`特殊性，为保证API较好的相容性，故将舵机两两一组，两个为一组进行初始化，最大可配置12路舵机输出。

* 舵机控制信号较为简单，但`MCPWM`数量较少。为了避免浪费`ESP32`的`MCPWM`资源，可以改用`LEDC`输出50Hz的PWM来控制舵机，这样一来，ESP32控制的舵机数量便可提升至12+16=28个。