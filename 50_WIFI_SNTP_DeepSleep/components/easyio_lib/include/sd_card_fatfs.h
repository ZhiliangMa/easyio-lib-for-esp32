#ifndef __SD_CARD_FATFS_H__
#define __SD_CARD_FATFS_H__

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#include "driver/sdmmc_host.h"
#endif

#define MOUNT_POINT "/sdcard"

// 该示例可以使用SDMMC和SPI外设与SD卡通信。
// 默认情况下，使用SDMMC外设。
// 要启用SPI模式，请取消注释以下行：

#define USE_SPI_MODE

// ESP32-S2没有SD主机外设，请始终使用SPI：
#ifdef CONFIG_IDF_TARGET_ESP32S2
#ifndef USE_SPI_MODE
#define USE_SPI_MODE
#endif // USE_SPI_MODE
// 在ESP32-S2上，DMA通道必须与主机ID相同
#define SPI_DMA_CHAN    host.slot
#endif //CONFIG_IDF_TARGET_ESP32S2

// SPI外设要使用的DMA通道
#ifndef SPI_DMA_CHAN
#define SPI_DMA_CHAN    1
#endif //SPI_DMA_CHAN

// 在测试SD和SPI模式时，请记住，一旦在SPI模式下初始化了卡，就无法在没有切换卡电源的情况下在SD模式下重新将其初始化。

#ifdef USE_SPI_MODE
// 使用SPI模式时的引脚映射通过此映射，SD卡既可以在SPI模式下使用，也可以在1线SD模式下使用。
// 请注意，在SD模式下，需要在CS线上上拉。
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   26
#endif //USE_SPI_MODE


/**
 * @brief  SD卡初始化、FATFS文件系统挂载
 * 		- 默认总线使用1/4线SDIO。如需SPI，需取消 .h文件中注释 // #define USE_SPI_MODE
 * 
 * @param  bus_width SDIO总线宽度。1线/4线
 * @param  bus_freq 总线速率。总线频率最大限制为40MHz
 * 
 * @return
 *     - sdmmc_card_t*：返回SDMMC卡指针，外部可由此获取卡信息。
 *     - 为0时则表示SD卡初始化及FATFS挂载失败
 */
sdmmc_card_t* sd_card_fatfs_init(uint8_t bus_width, uint16_t bus_freq);

/**
 * @brief  SD卡初始化、FATFS文件系统挂载
 * 		- 仅限于SPI接口模式，且SPI总线的初始化必须先于本函数。
 *      - 与先前初始化的SPI设备，共用同一SPI总线 和 DMA通道，默认使用VSPI_HOST的IO_MUX端口，默认CLK频率为20MHz。
 * 
 * @param  void
 * 
 * @return
 *     - sdmmc_card_t*：返回SDMMC卡指针，外部可由此获取卡信息。
 *     - 为0时则表示SD卡初始化及FATFS挂载失败
 */
sdmmc_card_t* sd_card_fatfs_spi_init(void);


#endif
