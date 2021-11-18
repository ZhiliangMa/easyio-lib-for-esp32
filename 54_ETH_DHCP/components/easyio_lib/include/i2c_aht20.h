#ifndef __I2C_AHT20_H__
#define __I2C_AHT20_H__

#include "i2c_config.h"
#include "esp_log.h"

#define AHT20_TICKS_TO_WAIT           (100 / portTICK_RATE_MS)    // I2C读写的超时等待时间

#define AHT20_SLAVE_ADDRESS           0x38    // AHT20在I2C总线上的从机器件地址

#define	AHT20_REG_STATUS              0x00    //状态字 寄存器地址
#define	AHT20_REG_INIT                0xBE    //初始化 寄存器地址
#define	AHT20_CMD_SoftReset           0xBA    //软复位 单指令
#define	AHT20_REG_TrigMeasure         0xAC    //触发测量 寄存器地址

// AHT20 状态、温湿度结构体
struct m_AHT20
{
	uint8_t alive;	// 0-器件不存在; 1-器件存在
	uint8_t flag;	// 读取/计算错误标志位。0-读取/计算数据正常; 1-读取/计算设备失败
	uint32_t HT[2];	// 湿度、温度 原始传感器的值，20Bit
	
	float RH;		// 湿度，转换单位后的实际值，标准单位%
	float Temp;		// 温度，转换单位后的实际值，标准单位°C
};


/**
 * @brief  AHT20 设备初始化
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * 
 * @return
 * 		- 0：AHT20 初始化成功。
 *		- 1：错误，AHT20设备初始化失败。
 * 		- 2：错误：总线上不存在 AHT20器件。
 */
uint8_t i2c_aht20_init(i2c_port_t i2c_num);

/**
 * @brief  AHT20 设备读取 相对湿度和温度（原始数据20Bit）
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @param  HT 存储20Bit原始数据的uint32数组
 * 
 * @return
 * 		- 0：读取数据正常
 *		- 1：读取设备失败，设备一直处于忙状态，不能获取数据
 */
uint8_t i2c_aht20_read_ht(i2c_port_t i2c_num, uint32_t *HT);

/**
 * @brief  AHT20 温湿度信号转换（由20Bit原始数据，转换为标准单位RH=%，T=°C）
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @param  aht 存储AHT20传感器信息的结构体
 * 
 * @return
 * 		- 0：计算数据正常
 *		- 1：计算数据失败，计算值超出元件手册规格范围
 */
uint8_t aht20_standard_unit_conversion(struct m_AHT20* aht);


#endif
