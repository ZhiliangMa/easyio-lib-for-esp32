#include "i2c_mpu6050.h"

static const char *TAG = "i2c-mpu6050";

// 适用于 MPU6050 的 I2C读
static void i2c_mpu6050_read(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data_rd, size_t size)
{
    i2c_master_read_slave_reg(i2c_num, MPU6050_SLAVE_ADDRESS, reg_addr, data_rd, size, MPU6050_TICKS_TO_WAIT);
}

// 适用于 MPU6050 的 I2C写
static void i2c_mpu6050_write(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data_wr, size_t size)
{
    i2c_master_write_slave_reg(i2c_num, MPU6050_SLAVE_ADDRESS, reg_addr, data_wr, size, MPU6050_TICKS_TO_WAIT);
}

/**
 * @brief  检测 MPU6050/9250（总线上是否存在 MPU6050/9250 器件？芯片是否是坏片？）
 *      - 检测MPU6050，读 WHO_AM_I 判断总线上是否有器件。再写入SMPLRT_DIV一个固定值，回读是否一致，判断芯片是否是坏片。
 *      - 检测坏片的原因：有的二手翻新元件，能读到WHO_AM_I，但不能对R/W属性的寄存器写入，导致不能正常工作。
 *      - 例：i2c_mpu6050_check(I2C_NUM_0);
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * 
 * @return
 *     - 0：成功。可继续初始化配置。
 *     - 1：错误，总线上不存在 MPU6050/9250 器件。
 *     - 2：错误，芯片坏片。
 */
static int32_t i2c_mpu6050_check(i2c_port_t i2c_num)
{
    uint8_t test = 0;

    // 读取 WHO_AM_I 寄存器，判断总线上是否存在 MPU6050/9250 器件
    i2c_mpu6050_read(i2c_num, MPU6050_REG_WHO_AM_I, &test, 1);
    // 打印器件型号
    switch (test) {
        case MPU6050_VAL_WHO_AM_I:
            ESP_LOGI(TAG, "WHO_AM_I Data: 0x%02x\tMPU6050", test);
            break;
        case MPU9250_VAL_WHO_AM_I:
            ESP_LOGI(TAG, "WHO_AM_I Data: 0x%02x\tMPU9250", test);
            break;
        case ICM20600_VAL_WHO_AM_I:
            ESP_LOGI(TAG, "WHO_AM_I Data: 0x%02x\tICM20600", test);
            break;
        case ICM20602_VAL_WHO_AM_I:
            ESP_LOGI(TAG, "WHO_AM_I Data: 0x%02x\tICM20602", test);
            break;
        case ICM20608_VAL_WHO_AM_I:
            ESP_LOGI(TAG, "WHO_AM_I Data: 0x%02x\tICM20608", test);
            break;
        default:
            ESP_LOGI(TAG, "WHO_AM_I Data: 0x%02x\tIIC%d BUS do not found MPU6050/9250 or ICM2060x device", test, i2c_num);
            return 1; // 返回错误：总线上不存在 MPU6050/9250 器件
    }

    // 向 SMPLRT_DIV 寄存器写 0x23 测试值，再读回看是否与写入的一致。判断芯片是否是坏片
    // 以下的检测坏片，对一些情况确实有用，但有时也会引起误判，建议屏蔽掉
    /*test = 0x23;
    i2c_mpu6050_write(i2c_num, MPU6050_REG_SMPLRT_DIV, &test, 1);
    vTaskDelay(20 / portTICK_PERIOD_MS); // 延时还是要加的，不然有几率会导致检查失败
    test = 0;
    i2c_mpu6050_read(i2c_num, MPU6050_REG_SMPLRT_DIV, &test, 1);
    if (test != 0x23) {
        ESP_LOGE(TAG, "read Data: 0x%02x\tMPU6050/9250 device is bad chip", test);
        return 2; // 返回错误：芯片坏片
    }*/

    ESP_LOGI(TAG, "IMU MPU6050/9250 or ICM2060x check succeed");
    return 0;
}

/**
 * @brief  配置 MPU6050/9250
 *      - 通过I2Cx总线写寄存器，来对功能进行配置。
 *      - 采样率500Hz，失能关闭，低通滤波带宽98Hz。加速度计量程：+-8g。陀螺仪量程：+-2000度/s
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * 
 * @return
 *     - none
 */
static void i2c_mpu6050_config(i2c_port_t i2c_num)
{
    uint8_t cfg_val = 0x00;
    // 开启时钟，失能睡眠，使能温度
    i2c_mpu6050_write(i2c_num, MPU6050_REG_PWR_MGMT_1, &cfg_val, 1);

    cfg_val = 0x01;
    // 采样频率500Hz
    i2c_mpu6050_write(i2c_num, MPU6050_REG_SMPLRT_DIV, &cfg_val, 1);

    cfg_val = MPU6050_DLPF_BW_98;
    // 失能中断引脚，设置低通滤波带宽98Hz
    i2c_mpu6050_write(i2c_num, MPU6050_REG_CONFIG, &cfg_val, 1);

    cfg_val = 0x10;
    // 加速度计量程：+-8g。0x18是+-16G
    i2c_mpu6050_write(i2c_num, MPU6050_REG_ACCEL_CONFIG, &cfg_val, 1);

    cfg_val = 0x18;
    // 陀螺仪量程：+-2000度/s
    i2c_mpu6050_write(i2c_num, MPU6050_REG_GYRO_CONFIG, &cfg_val, 1);
}

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
void i2c_mpu6050_init(i2c_port_t i2c_num)
{
    // 检测 MPU6050/9250（总线上是否存在 MPU6050/9250 器件？芯片是否是坏片？）
    while (i2c_mpu6050_check(i2c_num)) {
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    // 配置 MPU6050/9250
    // 采样率500Hz，失能关闭，低通滤波带宽98Hz。加速度计量程：+-8g。陀螺仪量程：+-2000度/s
    i2c_mpu6050_config(i2c_num);
}

/**
 * @brief  读取 MPU6050/9250 的原始 XYZ轴加速度计、温度、XYZ轴陀螺仪 数据
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @param  data_rd 读出的uint8_t数组的指针，存放读取出的数据，最少 14字节。
 * 
 * @return
 *     - none
 */
static void i2c_mpu6050_read_original_accel_temp_gyro(i2c_port_t i2c_num, uint8_t *data_rd)
{
    // 读取 XYZ轴加速度计、温度、XYZ轴陀螺仪数据。高字节在前，每个占2字节。
    i2c_mpu6050_read(i2c_num, MPU6050_REG_ACCEL_XOUT_H, data_rd, 14);
}

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
void i2c_mpu6050_get_accel_gyro(i2c_port_t i2c_num, int16_t *accel, int16_t *gyro)
{
    uint8_t original[14];
    i2c_mpu6050_read_original_accel_temp_gyro(i2c_num, original);

    *accel     = ((int16_t)original[0] << 8) | original[1];
    *(accel+1) = ((int16_t)original[2] << 8) | original[3];
    *(accel+2) = ((int16_t)original[4] << 8) | original[5];

    *gyro      = ((int16_t)original[8] << 8) | original[9];
    *(gyro+1)  = ((int16_t)original[10] << 8) | original[11];
    *(gyro+2)  = ((int16_t)original[12] << 8) | original[13];
}
