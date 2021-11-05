#ifndef __I2C_CONFIG_H__
#define __I2C_CONFIG_H__

#include "driver/i2c.h"

#define DATA_LENGTH 512                  /*!< Data buffer length of test buffer */

#define I2C_SLAVE_TX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave rx buffer size */

#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

#define I2Cx_SLAVE_ADDRESS 0x28


/**
 * @brief  配置I2Cx-主机模式，（I2C端口、总线速率、SCL引脚，SDA引脚）
 *      - 例：i2c_master_init(I2C_NUM_0, 100000, GPIO_NUM_18, GPIO_NUM_19);
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @param  clk_speed I2C总线速率。单位Hz，多使用 100000 400000
 * @param  scl_io_num SCL端口号。除仅能做输入 和 6、7、8、9、10、11之外的任意端口。
 * @param  sda_io_num SDA端口号
 * 
 * @return
 *     - none
 */
esp_err_t i2c_master_init(i2c_port_t i2c_num, uint32_t clk_speed, gpio_num_t scl_io_num, gpio_num_t sda_io_num);

/**
 * @brief  配置I2Cx-从机模式，（I2C端口、SCL引脚，SDA引脚）
 *      - 例：i2c_slave_init(I2C_NUM_0, I2Cx_SLAVE_ADDRESS, GPIO_NUM_18, GPIO_NUM_19);
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @param  slave_addr I2C作为从机时的器件地址
 * @param  scl_io_num SCL端口号。除仅能做输入 和 6、7、8、9、10、11之外的任意端口。
 * @param  sda_io_num SDA端口号
 * 
 * @return
 *     - none
 */
esp_err_t i2c_slave_init(i2c_port_t i2c_num, uint8_t slave_addr, gpio_num_t scl_io_num, gpio_num_t sda_io_num);

/**
 * @brief  I2Cx-读从设备的值
 *      - 不带有读器件寄存器的方式，适用于 BH1750、ADS1115/1118等少数I2C设备，这类设备通常内部寄存器很少
 *      - 例：i2c_master_read_slave(I2C_NUM_0, 0x68, &test, 1, 100 / portTICK_RATE_MS);
 *
 * ________________________________________________________________________________________
 * | start | slave_addr + rd_bit + ack | read n-1 bytes + ack | read 1 byte + nack | stop |
 * --------|---------------------------|----------------------|--------------------|------|
 *
 */
esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t *data_rd, size_t size, TickType_t ticks_to_wait);

/**
 * @brief  I2Cx-读从设备的寄存器值
 *      - 带有读器件寄存器的方式，适用于 MPU6050、ADXL345、HMC5983、MS5611、BMP280等绝大多数I2C设备
 *      - 例：i2c_master_read_slave_reg(I2C_NUM_0, 0x68, 0x75, &test, 1, 100 / portTICK_RATE_MS);
 * 
 * _____________________________________________________________________________________________________________________________________________
 * | start | slave_addr + rd_bit + ack | reg_addr + ack | start | slave_addr + wr_bit + ack | read n-1 bytes + ack | read 1 byte + nack | stop |
 * --------|---------------------------|------------------------|---------------------------|----------------------|--------------------|------|
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @param  slave_addr I2C读从机的器件地址
 * @param  reg_addr I2C读从机的寄存器地址
 * @param  data_rd 读出的值的指针，存放读取出的数据
 * @param  size 读取的寄存器数目
 * @param  ticks_to_wait 超时等待时间
 * 
 * @return
 *     - esp_err_t
 */
esp_err_t i2c_master_read_slave_reg(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t reg_addr, uint8_t *data_rd, size_t size, TickType_t ticks_to_wait);

/**
 * @brief  I2Cx-读从设备的寄存器值（寄存器地址 或 命令 为2字节的器件）
 *      - 带有读器件寄存器的方式，适用于 SHT20、GT911 这种寄存器地址为16位的I2C设备
 *      - 例：i2c_master_read_slave_reg_16bit(I2C_NUM_0, 0x44, 0xE000, &test, 6, 100 / portTICK_RATE_MS);
 * 
 * _____________________________________________________________________________________________________________________________________________
 * | start | slave_addr + rd_bit + ack | reg_addr(2byte) + ack | start | slave_addr + wr_bit + ack | read n-1 bytes + ack | read 1 byte + nack | stop |
 * --------|---------------------------|------------------------|---------------------------|----------------------|--------------------|------|
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @param  slave_addr I2C读从机的器件地址
 * @param  reg_addr I2C读从机的寄存器地址(2byte)
 * @param  data_rd 读出的值的指针，存放读取出的数据
 * @param  size 读取的寄存器数目
 * @param  ticks_to_wait 超时等待时间
 * 
 * @return
 *     - esp_err_t
 */
esp_err_t i2c_master_read_slave_reg_16bit(i2c_port_t i2c_num, uint8_t slave_addr, uint16_t reg_addr, uint8_t *data_rd, size_t size, TickType_t ticks_to_wait);

/**
 * @brief  I2Cx-写从设备的值
 *      - 不带有写器件寄存器的方式，适用于 BH1750、ADS1115/1118等少数I2C设备，这类设备通常内部寄存器很少
 *      - 例：i2c_master_write_slave(I2C_NUM_0, 0x68, &test, 1, 100 / portTICK_RATE_MS);
 *
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|
 *
 */
esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t *data_wr, size_t size, TickType_t ticks_to_wait);

/**
 * @brief  I2Cx-写从设备的寄存器值
 *      - 带有写器件寄存器的方式，适用于 MPU6050、ADXL345、HMC5983、MS5611、BMP280等绝大多数I2C设备
 *      - 例：i2c_master_write_slave_reg(I2C_NUM_0, 0x68, 0x75, &test, 1, 100 / portTICK_RATE_MS);
 * 
 * ____________________________________________________________________________________
 * | start | slave_addr + wr_bit + ack | reg_addr + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------|----------------------|------|
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @param  slave_addr I2C写从机的器件地址
 * @param  reg_addr I2C写从机的寄存器地址
 * @param  data_wr 写入的值的指针，存放写入进的数据
 * @param  size 写入的寄存器数目
 * @param  ticks_to_wait 超时等待时间
 * 
 * @return
 *     - esp_err_t
 */
esp_err_t i2c_master_write_slave_reg(i2c_port_t i2c_num, uint8_t slave_addr, uint8_t reg_addr, uint8_t *data_wr, size_t size, TickType_t ticks_to_wait);


#endif
