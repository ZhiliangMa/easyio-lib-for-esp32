#ifndef __I2C_MPU6050_H__
#define __I2C_MPU6050_H__

#include "i2c_config.h"
#include "esp_log.h"

#define MPU6050_TICKS_TO_WAIT           (100 / portTICK_RATE_MS)    // I2C读写的超时等待时间

#define MPU6050_SLAVE_ADDRESS           0x68    // MPU6050在I2C总线上的从机器件地址

#define MPU6050_REG_WHO_AM_I            0x75
#define MPU6050_REG_SELF_TEST_X_GYRO    0x00

#define MPU6050_REG_SMPLRT_DIV          0x19
#define MPU6050_REG_CONFIG              0x1A
#define MPU6050_REG_GYRO_CONFIG         0x1B
#define MPU6050_REG_ACCEL_CONFIG        0x1C
#define MPU6050_REG_ACCEL_CONFIG_2      0x1D
#define MPU6050_REG_PWR_MGMT_1          0x6B

#define MPU6050_REG_ACCEL_XOUT_H        0x3B
#define MPU6050_REG_ACCEL_XOUT_L        0x3C

#define MPU6050_DLPF_BW_256             0x00
#define MPU6050_DLPF_BW_188             0x01
#define MPU6050_DLPF_BW_98              0x02
#define MPU6050_DLPF_BW_42              0x03
#define MPU6050_DLPF_BW_20              0x04
#define MPU6050_DLPF_BW_10              0x05
#define MPU6050_DLPF_BW_5               0x06

#define MPU6050_VAL_WHO_AM_I            0x68    // MPU6050的 WHO_AM_I 寄存器默认值
#define MPU9250_VAL_WHO_AM_I            0x71    // MPU9250的 WHO_AM_I 寄存器默认值
#define ICM20600_VAL_WHO_AM_I           0x11    // ICM20600的 WHO_AM_I 寄存器默认值
#define ICM20602_VAL_WHO_AM_I           0x12    // ICM20602的 WHO_AM_I 寄存器默认值
#define ICM20608_VAL_WHO_AM_I           0xAE    // ICM20608的 WHO_AM_I 寄存器默认值


/**
 * @brief  初始化 MPU6050/9250，兼容ICM-20602/8
 *      - 检测 MPU6050/9250。并通过I2Cx总线写寄存器，来对功能进行配置。
 *      - 采样率500Hz，失能关闭，低通滤波带宽98Hz。加速度计量程：+-8g。陀螺仪量程：+-2000度/s
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * 
 * @return
 *     - none
 */
void i2c_mpu6050_init(i2c_port_t i2c_num);

/**
 * @brief  读取 MPU6050/9250 的 XYZ轴加速度计、XYZ轴陀螺仪 数据
 *      - 指针里依次存放 XYZ 轴数据
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @param  accel 读出【加速度计】的值的指针，存放XYZ轴加速度计数据。最少 3个int16_t（一定要int16_t，值是有正负号的）
 * @param  gyro 读出【陀螺仪】的值的指针，存放XYZ轴陀螺仪计数据。最少 3个int16_t（一定要int16_t，值是有正负号的）
 * 
 * @return
 *     - none
 */
void i2c_mpu6050_get_accel_gyro(i2c_port_t i2c_num, int16_t *accel, int16_t *gyro);


#endif
