# 26_I2C_MPU6050

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **i2c_mpu6050_task** ，读取 MPU6050的 XYZ轴加速度计、XYZ轴陀螺仪 数据。IC兼容 `MPU6050/9250`、`ICM-20600/2/8`

使用 `i2c_config.c.h` 驱动模块，对ESP32的 `I2C` 进行配置。

并用 `i2c_mpu6050.c.h` 驱动挂载在 `I2C` 总线上的`IMU`设备 `MPU6050/9250`、`ICM-20600/2/8`。


## 硬件连接

|                  | SCL    | SDA    |
| ---------------- | ------ | ------ |
| ESP32 I2C Master | GPIO14 | GPIO4 |
| MPU6050/9250     | SCL    | SDA    |

该驱动对6轴惯性测量单元，同时支持的型号有 `MPU6050/9250`、`ICM-20600/2/8`。


## 运行现象

* LED闪烁。

* 初始化 I2C 总线上的IMU设备，进行检查器件、功能配置，之后连续读加速度计/陀螺仪原始数据，并在终端中显示。

* 将开发板`水平`放置、`侧放`、`竖直`，观察加速度计的示数。


## 学习内容

1. I2C 总线时序。

2. IMU设备功能及应用场景。


## 关键函数

```c
// 配置I2Cx-主机模式，（I2C端口、总线速率、SCL引脚，SDA引脚）
esp_err_t i2c_master_init(i2c_port_t i2c_num, uint32_t clk_speed, gpio_num_t scl_io_num, gpio_num_t sda_io_num);

// 初始化 MPU6050/9250，兼容ICM-20602/8
void i2c_mpu6050_init(i2c_port_t i2c_num);

// 读取 MPU6050/9250 的 XYZ轴加速度计、XYZ轴陀螺仪 数据
void i2c_mpu6050_get_accel_gyro(i2c_port_t i2c_num, int16_t *accel, int16_t *gyro);
```


## 注意事项

* IMU的硬件电路，决定了其`I2C器件从机地址`。开发板的`SDO/AD0`默认接GND，器件地址为 0x68。

* 如果器件检测过程中报错，请检查芯片，可能已经损坏。

* 调试I2C设备，可借助例程 `25_I2C_TOOLS`，可快速验证硬件正确性。也可读取I2C从机的寄存器内容，辅助器件驱动的编写。

* 例程中设置的IMU加速度计量程为 `+-8G`，分辨率为 32768/8 LSB/g。陀螺仪量程为 `+-2000度/s`，分辨率为 32768/2000 LSB/(dps) 。如需更改，请修改 `i2c_mpu6050_config`。
