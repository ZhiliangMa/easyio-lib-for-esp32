#ifndef __SPI_AS5047P_H__
#define __SPI_AS5047P_H__

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define NOP_REGISTER         0x0000
#define ERRFL_REGISTER       0x0001
#define PROG_REGISTER        0x0003
#define DIAAGC_REGISTER      0x3FFC // 默认值 0x0180，会根据磁场强度自动变化，只有转子静止时值才会静止
#define MAG_REGISTER         0x3FFD
#define ANGLEUNC_REGISTER    0x3FFE // 没有动态角度误差补偿的角度信息
#define ANGLECOM_REGISTER    0x3FFF // 具有动态角度误差补偿的角度信息

#define bit_read    1
#define bit_write   0

// AS5047P与SPI关联的句柄，通过此来调用SPI总线上的AS5047P设备
extern spi_device_handle_t AS5047P_SPI;


#define AS5047P_SOFT_CS0  17  // 软件的CS0引脚（硬CS太快，易导致通信失败）


/**
 * @brief  读AS5047P角度的原始值（适用于只读一次的情况）
 *      - 例：angle = as5047p_read_angle_once(AS5047P_SPI, AS5047P_SOFT_CS0);
 * 
 * @param  spi AS5047P与SPI关联的句柄，通过此来调用SPI总线上的AS5047P设备
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - AS5047P的角度值，0~16383 => 0~360°（如角度返回错误，值会 >= 16384）
 */
uint16_t as5047p_read_angle_once(spi_device_handle_t spi, gpio_num_t cs_io_num);

/**
 * @brief  读AS5047P角度的原始值（适用于连续读的情况，需要注意当前的数据是上一帧命令的返回值）
 *      - 例：angle = as5047p_read_angle_continuous(AS5047P_SPI, AS5047P_SOFT_CS0);
 * 
 * @param  spi AS5047P与SPI关联的句柄，通过此来调用SPI总线上的AS5047P设备
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - AS5047P的角度值，0~16383 => 0~360°（如角度返回错误，值会 >= 16384）
 */
uint16_t as5047p_read_angle_continuous(spi_device_handle_t spi, gpio_num_t cs_io_num);

/**
 * @brief  将AS5047P角度的原始值转换为实际360°角度（如角度返回错误，值会 >= 360°）
 *      - 例：angle = as5047p_to_angle(data);
 * 
 * @param  data 输入读AS5047P角度的原始值
 * 
 * @return
 *     - AS5047P的实际角度值，0~16383 => 0~360°（如角度返回错误，值会 >= 360°）
 */
float as5047p_to_angle(uint16_t data);

/**
 * @brief  AS5047P初始化，初始化SPI总线，配置为 SPI mode 1.(CPOL=0, CPHA=1)，CS引脚使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 *      - AS5047P的初始化除了设置SPI总线，没有其他过程，不用配置寄存器。电后至少延时等待tpon=10ms。进行SPI通信即可。
 *      - 例：spi_as5047p_init(SPI3_HOST, 100*1000, AS5047P_SOFT_CS0);
 * 
 * @param  host_id SPI端口号。SPI1_HOST / SPI2_HOST / SPI3_HOST
 * @param  clk_speed AS5047P设备的SPI速度（注意：普通GPIO最大只能30MHz，而AS5047P的最快SPI速率为10MHz）
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - none
 */
void spi_as5047p_init(spi_host_device_t host_id, uint32_t clk_speed, gpio_num_t cs_io_num);


#endif
