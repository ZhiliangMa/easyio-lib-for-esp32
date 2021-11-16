#include "i2c_aht20.h"

static const char *TAG = "i2c-aht20";

// 适用于 AHT20 的 I2C读
static void i2c_aht20_read(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data_rd, size_t size)
{
    i2c_master_read_slave_reg(i2c_num, AHT20_SLAVE_ADDRESS, reg_addr, data_rd, size, AHT20_TICKS_TO_WAIT);
}

// 适用于 AHT20 的 I2C写
static void i2c_aht20_write(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data_wr, size_t size)
{
    i2c_master_write_slave_reg(i2c_num, AHT20_SLAVE_ADDRESS, reg_addr, data_wr, size, AHT20_TICKS_TO_WAIT);
}

/**
 * @brief  读AHT20 设备状态字
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @return
 *     - none
 */
static uint8_t i2c_aht20_read_status(i2c_port_t i2c_num)
{
	uint8_t tmp[1];
    i2c_aht20_read(i2c_num, AHT20_REG_STATUS, tmp, 1);
	return tmp[0];
}

/**
 * @brief  读AHT20 设备状态字 中的Bit3: 校准使能位
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @return
 *     - none
 */
static uint8_t i2c_aht20_read_cal_enable(i2c_port_t i2c_num)
{
	uint8_t tmp;
	tmp = i2c_aht20_read_status(i2c_num);
	return (tmp>>3)&0x01;
}

/**
 * @brief  读AHT20 设备状态字 中的Bit7: 忙标志
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @return
 *     - int8_t 忙标志：1 - 设备忙; 0 - 设备空闲
 */
static uint8_t i2c_aht20_read_busy(i2c_port_t i2c_num)
{
	uint8_t tmp;
	tmp = i2c_aht20_read_status(i2c_num);
	return (tmp>>7)&0x01;
}

/**
 * @brief  AHT20 芯片初始化命令
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @return
 *     - none
 */
static void i2c_aht20_ic_init(i2c_port_t i2c_num)
{
	uint8_t tmp[2];
	tmp[0] = 0x08;
	tmp[1] = 0x00;
    i2c_aht20_write(i2c_num, AHT20_REG_INIT, tmp, 2);
}

/**
 * @brief  AHT20 触发测量命令
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @return
 *     - none
 */
static void i2c_aht20_trig_measure(i2c_port_t i2c_num)
{
	uint8_t tmp[2];
	tmp[0] = 0x33;
	tmp[1] = 0x00;
    i2c_aht20_write(i2c_num, AHT20_REG_TrigMeasure, tmp, 2);
}

/**
 * @brief  AHT20 软复位命令
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * @return
 *     - none
 */
static void i2c_aht20_soft_reset(i2c_port_t i2c_num)
{
	uint8_t tmp[1];
    i2c_aht20_write(i2c_num, AHT20_CMD_SoftReset, tmp, 0);
}

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
uint8_t i2c_aht20_init(i2c_port_t i2c_num)
{
	uint8_t rcnt = 2+1;//软复位命令 重试次数，2次
	uint8_t icnt = 2+1;//初始化命令 重试次数，2次
	
	while(--rcnt)
	{
		icnt = 2+1;
		
		vTaskDelay(40 / portTICK_PERIOD_MS);//上电后要等待40ms

		// 检测 I2C总线上是否存在 AHT20器件
		uint8_t temp[1];
		int32_t err = i2c_master_read_slave_reg(i2c_num, AHT20_SLAVE_ADDRESS, AHT20_REG_STATUS, temp, 1, AHT20_TICKS_TO_WAIT);
		printf("temp: %d\terr: %d\n", temp[0], err);
		if(err<0){ // I2C读错误
			return 2;// 错误：总线上不存在 AHT20器件
		}

		// 读取温湿度之前，首先检查[校准使能位]是否为1
		while((!i2c_aht20_read_cal_enable(i2c_num)) && (--icnt))// 2次重试机会
		{
			vTaskDelay(10 / portTICK_PERIOD_MS);
			// 如果不为1，要发送初始化命令
			i2c_aht20_ic_init(i2c_num);
			vTaskDelay(200 / portTICK_PERIOD_MS);//这个时间不确定，手册没讲
		}
		
		if(icnt)//[校准使能位]为1,校准正常
		{
			break;//退出rcnt循环
		}
		else//[校准使能位]为0,校准错误
		{
			i2c_aht20_soft_reset(i2c_num);//软复位AHT20器件，重试
			vTaskDelay(200 / portTICK_PERIOD_MS);//这个时间不确定，手册没讲
		}
	}
	
	if(rcnt)
	{
		vTaskDelay(200 / portTICK_PERIOD_MS);//这个时间不确定，手册没讲
		return 0;// AHT20设备初始化正常
	}
	else
	{
		return 1;// AHT20设备初始化失败
	}
}

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
uint8_t i2c_aht20_read_ht(i2c_port_t i2c_num, uint32_t *HT)
{
	uint8_t cnt=3+1;//忙标志 重试次数，3次
	uint8_t tmp[6];
	uint32_t RetuData = 0;
	
	// 发送触发测量命令
	i2c_aht20_trig_measure(i2c_num);
	
	do{
		vTaskDelay(75 / portTICK_PERIOD_MS);//等待75ms待测量完成，忙标志Bit7为0
	}while(i2c_aht20_read_busy(i2c_num) && (--cnt));//重试3次
	
	if(cnt)//设备闲，可以读温湿度数据
	{
		vTaskDelay(5 / portTICK_PERIOD_MS);
		// 读温湿度数据
        i2c_aht20_read(i2c_num, AHT20_REG_STATUS, tmp, 6);
		// 计算相对湿度RH。原始值，未计算为标准单位%。
		RetuData = 0;
		RetuData = (RetuData|tmp[1]) << 8;
		RetuData = (RetuData|tmp[2]) << 8;
		RetuData = (RetuData|tmp[3]);
		RetuData = RetuData >> 4;
		HT[0] = RetuData;
		
		// 计算温度T。原始值，未计算为标准单位°C。
		RetuData = 0;
		RetuData = (RetuData|tmp[3]) << 8;
		RetuData = (RetuData|tmp[4]) << 8;
		RetuData = (RetuData|tmp[5]);
		RetuData = RetuData&0xfffff;
		HT[1] = RetuData;
		
		return 0;
	}
	else//设备忙，返回读取失败
	{
		return 1;
	}
}

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
uint8_t aht20_standard_unit_conversion(struct m_AHT20* aht)
{
	aht->RH = (double)aht->HT[0] *100 / 1048576;//2^20=1048576 //原式：(double)aht->HT[0] / 1048576 *100，为了浮点精度改为现在的
	aht->Temp = (double)aht->HT[1] *200 / 1048576 -50;
	
	//限幅,RH=0~100%; Temp=-40~85°C
	if((aht->RH >=0)&&(aht->RH <=100) && (aht->Temp >=-40)&&(aht->Temp <=85))
	{
		aht->flag = 0;
		return 0;//测量数据正常
	}
	else
	{
		aht->flag = 1;
		return 1;//测量数据超出范围，错误
	}
}
