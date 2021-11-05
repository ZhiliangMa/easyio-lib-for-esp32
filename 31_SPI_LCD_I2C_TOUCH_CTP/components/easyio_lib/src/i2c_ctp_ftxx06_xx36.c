#include "i2c_ctp_ftxx06_xx36.h"

static const char *TAG = "i2c-ctp-FTxxxx";

// 适用于 FTxxxx 的 I2C读
static void i2c_ctp_FTxxxx_read(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data_rd, size_t size)
{
    i2c_master_read_slave_reg(i2c_num, CTP_FTxxxx_SLAVE_ADDRESS, reg_addr, data_rd, size, CTP_FTxxxx_TICKS_TO_WAIT);
}

// 适用于 FTxxxx 的 I2C写
static void i2c_ctp_FTxxxx_write(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data_wr, size_t size)
{
    i2c_master_write_slave_reg(i2c_num, CTP_FTxxxx_SLAVE_ADDRESS, reg_addr, data_wr, size, CTP_FTxxxx_TICKS_TO_WAIT);
}

/*////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief  检测 FTxxxx（总线上是否存在 FTxxxx 器件？）
 *      - 例：i2c_ctp_FTxxxx_check(I2C_NUM_0);
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * 
 * @return
 *     - 0：成功。可继续初始化配置。
 *     - 1：错误，总线上不存在 CTP FTxxxx 器件。
 */
static int32_t i2c_ctp_FTxxxx_check(i2c_port_t i2c_num)
{
    uint8_t test = 0;

    // 读取 CTP_FTxxxx_CIPHER 寄存器，判断总线上是否存在 FTxxxx 器件
    i2c_ctp_FTxxxx_read(i2c_num, CTP_FTxxxx_CIPHER, &test, 1);
    
    if (test==0) {
        ESP_LOGE(TAG, "IIC%d BUS do not found CTP FTxxxx device,ERR: 0x%02X", i2c_num, test);
        return 1;
    } else {
        // 读到的值不为0，则证明I2C总线上存在器件
        ESP_LOGI(TAG, "Find CTP Chip vendor ID: 0x%02X", test);
        return 0;
    }
}

/**
 * @brief  FTxxxx触控芯片初始化。（将 DEVIDE_MODE 写0，使IC处于工作模式即可。其他参数默认。）
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * 
 * @return
 *     - none
 */
void i2c_ctp_FTxxxx_init(i2c_port_t i2c_num)
{
    // 检测总线上是否存在 FTxxxx 器件？
    while (i2c_ctp_FTxxxx_check(i2c_num)) {
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    // 配置 CTP FTxxxx
	uint8_t tmp[1] = {0x00}; // Bit[6:4]，为0是工作模式；0x100为工厂模式
    // 设置为正常工作模式
    i2c_ctp_FTxxxx_write(i2c_num, CTP_FTxxxx_DEVIDE_MODE, tmp, 1);
    tmp[0] = 0x00; // 0x00-中断轮询模式；0x01-中断触发模式
    // 设置为中断轮询模式
    i2c_ctp_FTxxxx_write(i2c_num, CTP_FTxxxx_MODE, tmp, 1);
    ESP_LOGI(TAG, "CTP FTxxxx Configure success!");
}

/**
 * @brief  读取触摸发生的总个数（通过读FTxxxx 触摸状态寄存器，从中提取出产生触摸的个数。）
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * 
 * @return
 *     - 产生触摸的个数。无触摸时为0。
 */
uint8_t i2c_ctp_FTxxxx_read_tp_num(i2c_port_t i2c_num)
{
	uint8_t tmp[1];
    i2c_ctp_FTxxxx_read(i2c_num, CTP_FTxxxx_TD_STATUS, tmp, 1);
    // 读取触摸发生的总个数。处理异常，刚上电后无触摸会读取出 0xFF。有触摸后值正常。
    if (tmp[0]==0xFF) {
        tmp[0] = 0;
    }
	return tmp[0];
}

/**
 * @brief  读FTxxxx 所有触摸信息。（触摸点个数、坐标、事件、ID）
 *      - 一次性从 0x02开始读，每次读 1+6*5=31 字节，里面包含5个触摸点的信息。
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * 
 * @return
 *     - none
 */
void i2c_ctp_FTxxxx_read_all(i2c_port_t i2c_num, ctp_tp_t* ctp)
{
	uint8_t tmp[31];
    uint16_t pos;
    // 一次性从 0x02开始读，每次读 1+6*5=31 字节，里面包含5个触摸点的信息。
    i2c_ctp_FTxxxx_read(i2c_num, CTP_FTxxxx_TD_STATUS, tmp, 31);

    // 从31字节中，解析触摸发生的总个数
    if (tmp[0]==0xFF) { // 读取触摸发生的总个数。处理异常，刚上电后无触摸会读取出 0xFF。有触摸后值正常。
        tmp[0] = 0;
    }
    if (tmp[0]>5) { //限幅，触摸不超过5个点
        tmp[0] = 5;
    }
    ctp->tp_num = tmp[0]; // 触摸发生的总个数
    // 从31字节中，根据总个数，解析各点坐标及事件类型
    for(uint8_t i=0; i<tmp[0]; i++) {
        ctp->tp[i].event = tmp[1+i*6] >> 6;    // 事件类型：0-按下；1-抬起；2-接触/长按；3-无事件
        ctp->tp[i].id = tmp[3+i*6] >> 4;       // 触摸ID
        ctp->tp[i].x = (((uint16_t)tmp[1+i*6] & 0x0F) << 8) | tmp[2+i*6]; // 触摸产生的点的X坐标
        ctp->tp[i].y = (((uint16_t)tmp[3+i*6] & 0x0F) << 8) | tmp[4+i*6]; // 触摸产生的点的Y坐标

        // 根据电容触摸屏与LCD的安装。匹配方向
        // 置换X、Y的值
        pos = ctp->tp[i].y;
        ctp->tp[i].y = ctp->tp[i].x;
        ctp->tp[i].x = pos;
        // 调转X的方向
        ctp->tp[i].x = 480 -1 -ctp->tp[i].x;
    }
}
