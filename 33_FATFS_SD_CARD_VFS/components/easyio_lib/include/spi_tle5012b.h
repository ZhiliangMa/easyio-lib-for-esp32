#ifndef __SPI_TLE5012B_H__
#define __SPI_TLE5012B_H__

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"


// 读TLE5012B寄存器命令
#define CMD_READ_ANGLE        0x8021
#define CMD_READ_SPEED        0x8031

// TLE5012B与SPI关联的句柄，通过此来调用SPI总线上的TLE5012B设备
extern spi_device_handle_t TLE5012B_SPI;

#define TLE5012B_SOFT_CS0  17  // 软件的CS0引脚


/**
 * @brief  读TLE5012B角度的原始值（适用于只读一次的情况）
 *      - 例：xdata = tle5012b_read_angle(TLE5012B_SPI, TLE5012B_SOFT_CS0);
 * 
 * @param  spi TLE5012B与SPI关联的句柄，通过此来调用SPI总线上的TLE5012B设备
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - TLE5012B的角度值原始值，0~32767 => 0~360°.
 */
uint16_t tle5012b_read_angle(spi_device_handle_t spi, gpio_num_t cs_io_num);

/**
 * @brief  读TLE5012B速度的原始值（适用于只读一次的情况）
 *      - 例：xdata = tle5012b_read_speed(TLE5012B_SPI, TLE5012B_SOFT_CS0);
 * 
 * @param  spi TLE5012B与SPI关联的句柄，通过此来调用SPI总线上的TLE5012B设备
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - TLE5012B的速度值原始值。
 */
uint16_t tle5012b_read_speed(spi_device_handle_t spi, gpio_num_t cs_io_num);

/**
 * @brief  将TLE5012B角度的原始值转换为实际360°角度
 *      - 例：angle = tle5012_to_angle(data);
 * 
 * @param  data 输入读TLE5012B角度的原始值
 * 
 * @return
 *     - TLE5012B的实际角度值，0~32767 => 0~360°.
 */
float tle5012_to_angle(uint16_t data);

/**
 * @brief  TLE5012B初始化，初始化SPI总线，配置为 SPI mode 1.(CPOL=0, CPHA=1)，CS引脚使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 *      - TLE5012B的初始化除了设置SPI总线，没有其他过程，不用配置寄存器。电后至少延时等待tpon=10ms。进行SPI通信即可。
 *      - 例：spi_tle5012b_init(SPI3_HOST, 4*1000*1000, TLE5012B_SOFT_CS0);
 * 
 * @param  host_id SPI端口号。SPI1_HOST / SPI2_HOST / SPI3_HOST
 * @param  clk_speed TLE5012B设备的SPI速度（注意：普通GPIO最大只能30MHz，而TLE5012B的最快SPI速率为8MHz）
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致TLE5012B通信不正常）
 * 
 * @return
 *     - none
 */
void spi_tle5012b_init(spi_host_device_t host_id, uint32_t clk_speed, gpio_num_t cs_io_num);

#endif
