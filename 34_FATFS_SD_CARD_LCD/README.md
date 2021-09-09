# 34_FATFS_SD_CARD_LCD

## 例程简介

前几个Demo如 `28_SPI_LCD_ILI9341_9488_9481_ST7735_7789_7796_HX8357C`、`33_FATFS_SD_CARD`，都是在SPI总线上挂载了单个外设，并没有多个共用。

而由于`ESP32`的可用`GPIO`较少，必须充分利用IO资源。显然像前几个Demo中，单个外设独占总线的用法很浪费。

本例Demo的 `LCD` 与 `SD_CARD` 使用同一SPI总线，通过分时复用的方式，达到节省GPIO资源占用的目的。

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **spi_lcd_sd_card_fatfs_task** ，初始化 SPI总线、LCD、SD卡、FATFS文件系统，创建、重命名、写入、读取文件


## 硬件连接

`LCD` 与 `SD_CARD` 使用同一SPI总线，`VSPI_HOST`（ESP32的SPI3）。

ESP32 pin     | SPI pin | SD card pin | LCD pin |
--------------|---------|-------------|---------|
GPIO18        | SCK     | CLK         | SCL     |
GPIO23        | MOSI    | CMD         | SDA     |
GPIO19        | MISO    | D0          |         |
N/C           |         | D1          |         |
N/C           |         | D2          |         |
GPIO27        | CS_SD   | D3          |         |
GPIO5         | CS_LCD  |             | CS      |
EN            |         |             | RESET   |
GPIO22        |         |             | D/C     |
GPIO21        |         |             | BLK     |


## SPI的CLK速率测试（LCD与SD_CARD的CLK设置为不同频率）

`LCD` 与 `SD_CARD` 共用同一SPI总线，但两个外设的CLK频率明显不一样。`LCD`需要快速刷屏，要尽可能地快；而`SD卡`的SPI最大只能25MHz。

那么如果挂载的多个外设的CLK频率不一致，ESP32实际的输出效果是怎样的呢？通过示波器捕获观察实际频率：

* 设置`SPI`的`CLK`频率为 `40MHz`。LCD与SD分时复用，示波器实际测量结果为：`LCD`刷屏时CLK频率为`40MHz`；`SD`读写为`20MHz`。

* 设置`SPI`的`CLK`频率为 `80MHz`。LCD与SD分时复用，示波器实际测量结果为：`LCD`刷屏时CLK频率为`80MHz`；`SD`读写为`20MHz`。

故不同外设挂载于同一总线上，完全不需要担心频率不一致导致的使用问题，SDK会根据当前使用的设备，输出对应的匹配频率。

上面的`SD`读写时，频率一直为`20MHz`，是因为`SD卡`在SPI模式下最大频率仅为`25MHz`。而在IDF的配置中，`SD-SPI`的默认频率为`20MHz`。


## 运行现象

* LED闪烁。

* 打开终端，ESP32上电运行后，会初始化 SPI总线、LCD、SD卡、FATFS文件系统，创建、重命名、写入、读取文件。终端会显示出ESP32挂载的SD卡名称。LCD会显示终端输出的相同内容。


## 为什么使用SPI？而不适用SDIO

SDIO-1线模式，所用的IO数，比SPI模式还要少一根，且速率比SPI还快。

但因为还要另外挂载SPI-LCD，为了节约引脚占用，故与LCD使用同一种总线，SPI。


## 关键函数

```c
// SD卡初始化、FATFS文件系统挂载（仅限于SPI接口模式，且SPI总线的初始化必须先于本函数。）
sdmmc_card_t* sd_card_fatfs_spi_init(void);
```

乐鑫`ESP32-FATFS`帮助文档：[ESP32-FATFS](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/storage/fatfs.html)

函数功能及更多介绍详见官网在线文档。
`FATFS`官网：[FATFS在线文档](http://www.elm-chan.org/fsw/ff/00index_e.html)


## 注意事项

* SD卡的Block为4K，而FATFS最小也需要4K的缓存，所以创建任务时要多留出一些内存，否则会反复重启。

* 本Demo中，LCD 和 SD卡 共用同一SPI总线的同时，也共用了同一DMA通道。


## 如出现不能下载代码的现象

GPIO2引脚用作自举引脚，应为低电平才能进入UART下载模式。一种方法是使用跳线连接GPIO0和GPIO2，然后，大多数开发板上的自动复位电路会在进入下载模式时将GPIO2和GPIO0一起拉为低电平。所以设计PCB时尽量别用GPIO2做SD卡的引脚，建议做输出引脚，如LED输出。（编译官方sd_card后，烧录，很大概率会出现不能自动烧录的情况，就是GPIO2被拉高的原因，需要将GPIO2浮空或者手动拉低）

Connecting........_____....._____....._____....._____....._____....._____....._____

断开SD卡D0 / MISO线与GPIO2的连接，然后尝试再次上传。 阅读上面的“关于GPIO2的注意事项”。
