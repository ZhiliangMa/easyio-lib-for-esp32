#include "sd_card_fatfs.h"

static const char *TAG = "sd";

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
sdmmc_card_t* sd_card_fatfs_init(uint8_t bus_width, uint16_t bus_freq)
{
esp_err_t ret;
    // 用于挂载文件系统的选项。
    // 如果format_if_mount_failed设置为true，当SD卡挂载失败时，SD卡将被分区并格式化。
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t* card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    // 使用上面定义的设置初始化SD卡并挂载FAT文件系统。
    // 注意：esp_vfs_fat_sdmmc / sdspi_mount是多合一的便捷功能。
#ifndef USE_SPI_MODE
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // 这个初始化没有卡检测信号（CD）和写保护（WP）信号。
    // 如果您的主板有这些信号，请修改slot_config.gpio_cd和slot_config.gpio_wp。
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // 修改SDIO总线宽度。1线/4线
    slot_config.width = bus_width;
    // 修改总线速率。总线频率最大限制为40MHz
    host.max_freq_khz = bus_freq; // SDMMC_FREQ_HIGHSPEED / SDMMC_FREQ_DEFAULT

    ESP_LOGI(TAG, "%d Bit Mode", bus_width);

    // GPIO 15、2、4、12、13应该具有外部10k上拉电阻。
    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

    // 使用SDIO的方式驱动SD卡，并挂载FATFS文件系统
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
#else
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    host.slot = VSPI_HOST; // 重新配置SD卡的SPI端口

    // SPI模式下，因 SD卡在SPI模式下最大频率仅为25MHz，故配置的值不能超过此项。而在IDF的配置中，SD-SPI的默认频率为20MHz。

    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return false;
    }

    // 这个初始化没有卡检测信号（CD）和写保护（WP）信号。
    // 如果您的主板有这些信号，请修改slot_config.gpio_cd和slot_config.gpio_wp。
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    // 使用SPI的方式驱动SD卡，并挂载FATFS文件系统
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
#endif //USE_SPI_MODE

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return false;
    }

    // 卡已初始化，打印其属性
    sdmmc_card_print_info(stdout, card);

    /*// 使用POSIX和C标准库函数来处理文件。
    // 首先创建一个文件。
    ESP_LOGI(TAG, "Opening file");
    FILE* f = fopen(MOUNT_POINT"/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    // 重命名前检查目标文件是否存在
    struct stat st;
    if (stat(MOUNT_POINT"/foo.txt", &st) == 0) {
        // 删除（如果存在）
        unlink(MOUNT_POINT"/foo.txt");
    }

    // 重命名原始文件
    ESP_LOGI(TAG, "Renaming file");
    if (rename(MOUNT_POINT"/hello.txt", MOUNT_POINT"/foo.txt") != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // 打开重命名的文件以供阅读
    ESP_LOGI(TAG, "Reading file");
    f = fopen(MOUNT_POINT"/foo.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // 换行符
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    // 完成所有操作，卸载分区并禁用SDMMC或SPI外设
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");
#ifdef USE_SPI_MODE
    // 卸载所有设备后取消初始化总线
    spi_bus_free(host.slot);
#endif*/

    return card; // 返回SDMMC卡指针，外部可由此获取卡信息
}

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
sdmmc_card_t* sd_card_fatfs_spi_init(void)
{
esp_err_t ret;
    // 用于挂载文件系统的选项。
    // 如果format_if_mount_failed设置为true，当SD卡挂载失败时，SD卡将被分区并格式化。
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t* card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    /*spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };*/

    host.slot = VSPI_HOST; // 重新配置SD卡的SPI端口

    // SPI模式下，因 SD卡在SPI模式下最大频率仅为25MHz，故配置的值不能超过此项。而在IDF的配置中，SD-SPI的默认频率为20MHz。

    /*ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return false;
    }*/

    // 这个初始化没有卡检测信号（CD）和写保护（WP）信号。
    // 如果您的主板有这些信号，请修改slot_config.gpio_cd和slot_config.gpio_wp。
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    // 使用SPI的方式驱动SD卡，并挂载FATFS文件系统
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return false;
    }

    // 卡已初始化，打印其属性
    sdmmc_card_print_info(stdout, card);

    return card; // 返回SDMMC卡指针，外部可由此获取卡信息
}
