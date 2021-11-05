#include "i2c_sht30.h"

// SHT30驱动源码参考，特此感谢：https://cloud.tencent.com/developer/article/1662638

/**
 * @brief    向SHT30发送一条指令(16bit)
 * 
 * @param    cmd —— SHT30指令（在SHT30_MODE中枚举定义）
 * 
 * @retval    成功返回HAL_OK(ESP_OK)
*/
static uint8_t SHT30_Send_Cmd(SHT30_CMD cmd)
{
    uint8_t cmd_buffer[2];
    cmd_buffer[0] = cmd >> 8;
    cmd_buffer[1] = cmd;
    return i2c_master_write_slave_reg(SHT30_I2C_BUS, SHT30_SLAVE_ADDRESS, cmd_buffer[0], cmd_buffer+1, 1, SHT30_TICKS_TO_WAIT);
}

// SHT30读数据使用 i2c_master_read_slave_reg_16bit(i2c_port_t i2c_num, uint8_t slave_addr, uint16_t reg_addr, uint8_t *data_rd, size_t size, TickType_t ticks_to_wait);

/**
 * @brief    复位SHT30
 * 
 * @param    none
 * 
 * @retval    none
*/
void SHT30_reset(void)
{
    SHT30_Send_Cmd(SOFT_RESET_CMD);
    vTaskDelay(20 / portTICK_PERIOD_MS);
}

/**
 * @brief    初始化SHT30
 * 
 * @param    none
 * 
 * @retval    成功返回HAL_OK(ESP_OK)
 * 
 * @note    周期测量模式
*/
uint8_t SHT30_Init(void)
{
    return SHT30_Send_Cmd(MEDIUM_2_CMD);
}

/**
 * @brief    从SHT30读取一次数据
 * 
 * @param    dat —— 存储读取数据的地址（6个字节数组）
 * 
 * @retval    成功 —— 返回HAL_OK(ESP_OK)
*/
uint8_t SHT30_Read_Dat(uint8_t* dat)
{
    return i2c_master_read_slave_reg_16bit(SHT30_I2C_BUS, SHT30_SLAVE_ADDRESS, READOUT_FOR_PERIODIC_MODE, dat, 6, SHT30_TICKS_TO_WAIT);
}

#define CRC8_POLYNOMIAL 0x31

static uint8_t CheckCrc8(uint8_t* const message, uint8_t initial_value)
{
    uint8_t  remainder;        //余数
    uint8_t  i = 0, j = 0;  //循环变量

    /* 初始化 */
    remainder = initial_value;

    for(j = 0; j < 2;j++)
    {
        remainder ^= message[j];

        /* 从最高位开始依次计算  */
        for (i = 0; i < 8; i++)
        {
            if (remainder & 0x80)
            {
                remainder = (remainder << 1)^CRC8_POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    /* 返回计算的CRC码 */
    return remainder;
}

/**
 * @brief    将SHT30接收的6个字节数据进行CRC校验，并转换为温度值和湿度值
 * 
 * @param    dat  —— 存储接收数据的地址（6个字节数组）
 * 
 * @retval    校验成功  —— 返回0
 *            校验失败  —— 返回1，并设置温度值和湿度值为0
*/
uint8_t SHT30_Dat_To_Float(uint8_t* const dat, float* temperature, float* humidity)
{
    uint16_t recv_temperature = 0;
    uint16_t recv_humidity = 0;

    /* 校验温度数据和湿度数据是否接收正确 */
    if(CheckCrc8(dat, 0xFF) != dat[2] || CheckCrc8(&dat[3], 0xFF) != dat[5])
        return 1;

    /* 转换温度数据 */
    recv_temperature = ((uint16_t)dat[0]<<8)|dat[1];
    *temperature = -45 + 175*((float)recv_temperature/65535);

    /* 转换湿度数据 */
    recv_humidity = ((uint16_t)dat[3]<<8)|dat[4];
    *humidity = 100 * ((float)recv_humidity / 65535);

    return 0;
}
