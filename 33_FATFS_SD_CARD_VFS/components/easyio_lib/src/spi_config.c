#include "spi_config.h"


/**
 * @brief  配置SPIx主机模式，配置DMA通道、DMA字节大小，及 MISO、MOSI、CLK的引脚。
 *      - （注意：普通GPIO最大只能30MHz，而IOMUX默认的SPI-IO，CLK最大可以设置到80MHz）
 *      - 例：spi_master_init(SPI2_HOST, LCD_DEF_DMA_CHAN, LCD_DMA_MAX_SIZE, SPI2_DEF_PIN_NUM_MISO, SPI2_DEF_PIN_NUM_MOSI, SPI2_DEF_PIN_NUM_CLK);
 * 
 * @param  host_id SPI端口号。SPI1_HOST / SPI2_HOST / SPI3_HOST
 * @param  dma_chan 使用的DMA通道
 * @param  max_tran_size DMA最大的传输字节数（会根据此值给DMA分配内存，值越大分配给DMA的内存就越大，单次可用DMA传输的内容就越多）
 * @param  miso_io_num MISO端口号。除仅能做输入 和 6、7、8、9、10、11之外的任意端口，但仅IOMUX默认的SPI-IO才能达到最高80MHz上限。
 * @param  mosi_io_num MOSI端口号
 * @param  clk_io_num CLK端口号
 * 
 * @return
 *     - none
 */
void spi_master_init(spi_host_device_t host_id, int dma_chan, uint32_t max_tran_size, gpio_num_t miso_io_num, gpio_num_t mosi_io_num, gpio_num_t clk_io_num)
{
    esp_err_t ret;
    // 配置 MISO、MOSI、CLK、CS 的引脚，和DMA最大传输字节数
    spi_bus_config_t buscfg={
        .miso_io_num=miso_io_num,              // MISO引脚定义
        .mosi_io_num=mosi_io_num,              // MOSI引脚定义
        .sclk_io_num=clk_io_num,               // CLK引脚定义
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=max_tran_size,        // 最大传输字节数
    };

    // 初始化SPI总线
    ret=spi_bus_initialize(host_id, &buscfg, dma_chan);
    ESP_ERROR_CHECK(ret);
}
