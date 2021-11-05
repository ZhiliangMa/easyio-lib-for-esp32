#ifndef __I2C_CTP_FTXX06_XX36_H__
#define __I2C_CTP_FTXX06_XX36_H__

#include "i2c_config.h"
#include "esp_log.h"

// 敦泰的触摸控制IC，型号包括：FT5206、FT5336、FT5436、FT6206、FT6236，驱动几乎都是一样（区别是触摸点数、刷新率不同）
#define CTP_FTxxxx_TICKS_TO_WAIT       (10 / portTICK_RATE_MS)    // I2C读写的超时等待时间

#define CTP_FTxxxx_SLAVE_ADDRESS       0x38    // FTxxxx在I2C总线上的从机器件地址(敦泰的触摸控制IC，型号包括：FT5206、FT5336、FT5436、FT6206、FT6236)

#define	CTP_FTxxxx_DEVIDE_MODE         0x00    // 模式控制寄存器
#define	CTP_FTxxxx_TD_STATUS           0x02    // 触摸状态寄存器
#define	CTP_FTxxxx_P1_XH               0x03    // 第一个触摸点的X信息寄存器
#define	CTP_FTxxxx_P1_YH               0x05    // 第一个触摸点的Y信息寄存器
#define	CTP_FTxxxx_P1_WEIGHT           0x07    // 第一个触摸点的权重寄存器（一般不用）
#define	CTP_FTxxxx_P1_MISC             0x08    // 第一个触摸点的其他信息寄存器（一般不用）

#define	CTP_FTxxxx_P2_XH               0x09    // 第一个触摸点的寄存器
// 接下来以此类推，根据IC支持的触摸点数多少，决定有多少组这些寄存器。
// 一般MCU读取时读取5组长度大小即可。也就是一次性从 0x02开始读，每次读 1+6*5=31 字节。
// 或者分两次，先读 0x02。确定有触摸存在，再根据点数读相符数量的几组寄存器。

#define	CTP_FTxxxx_LIB_VER_H           0xA1    // 固件库版本高字节
#define	CTP_FTxxxx_CIPHER              0xA3    // 芯片供应商编号
#define	CTP_FTxxxx_MODE                0xA4    // 中断模式控制寄存器：0x00-中断轮询模式；0x01-中断触发模式
#define	CTP_FTxxxx_FIRMID              0xA6    // 固件版本

/**
 * @brief  触摸IC的信息结构体，方便管理触摸发生的5个点信息
 */
typedef struct {
    uint8_t event; // 事件类型：0-按下；1-抬起；2-接触/长按；3-无事件
    uint8_t id; // 触摸ID
    uint16_t x; // 触摸产生的点的X坐标
    uint16_t y; // 触摸产生的点的Y坐标
} ctp_tp_info_t;

typedef struct {
    uint8_t tp_num; // 发生触摸的点数量
    ctp_tp_info_t tp[5];
} ctp_tp_t;



/**
 * @brief  FTxxxx触控芯片初始化。（将 DEVIDE_MODE 写0，使IC处于工作模式即可。其他参数默认。）
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * 
 * @return
 *     - none
 */
void i2c_ctp_FTxxxx_init(i2c_port_t i2c_num);

/**
 * @brief  读FTxxxx 所有触摸信息。（触摸点个数、坐标、事件、ID）
 *      - 一次性从 0x02开始读，每次读 1+6*5=31 字节，里面包含5个触摸点的信息。
 * 
 * @param  i2c_num I2C端口号。I2C_NUM_0 / I2C_NUM_1
 * 
 * @return
 *     - none
 */
void i2c_ctp_FTxxxx_read_all(i2c_port_t i2c_num, ctp_tp_t* ctp);


#endif
