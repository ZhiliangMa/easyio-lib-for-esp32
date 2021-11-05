#ifndef __LCD_SPI_DMA_H__
#define __LCD_SPI_DMA_H__


#include "driver/spi_master.h"
#include "esp_log.h"
#include "spi_lcd.h"
#include "simple_gui.h"
#include <string.h>

/**
 * @brief  初始化lcd使用`SPI-DMA 双缓冲环形队列`刷屏（申请DMA双缓冲内存）
 *      - 会根据"spi_lcd.h"中定义的屏幕尺寸，自动匹配行像素的值，并根据行像素大小申请内存，所以不需要传入参数）
 * 
 * @param  void
 * 
 * @return
 *     - void
 */
void lcd_spi_dma_display_init(void);

/**
 * @brief  刷一整个LCD屏幕（传输使用DMA环形队列加速）
 *      - DMA双缓冲，可以充分利用CPU资源；分行传输，可以减少RAM占用
 * 
 * @param  spi SPI总线与LCD设备关联的句柄
 * @param  pixels 用于刷新屏幕GRAM的RGB565像素内容的指针
 * 
 * @return
 *     - void
 */
void lcd_spi_dma_display_img(spi_device_handle_t spi, uint16_t ***pixels);

/**
 * @brief  使用波动动效刷一整个LCD屏幕（传输使用DMA环形队列）并在lcd右下角显示帧率
 *      - 在lcd_spi_dma_display_img的基础上，添加了波动动效
 * 
 * @param  spi SPI总线与LCD设备关联的句柄
 * @param  pixels 用于刷新屏幕GRAM的RGB565像素内容的指针
 * 
 * @return
 *     - void
 */
void lcd_spi_dma_display_pretty_colors(spi_device_handle_t spi, uint16_t ***pixels);

#endif
