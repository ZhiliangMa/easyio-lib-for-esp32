# 33_FATFS_SD_CARD

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **sd_card_fatfs_task** ，初始化 SD卡、FATFS文件系统，创建、重命名、写入、读取文件


使用“多合一” esp_vfs_fat_sdmmc_mount函数可以：

* 初始化SDMMC外设，

* 探测并初始化连接到SD / MMC插槽1（HS2_CMD，HS2_CLK，HS2_D0，HS2_D1，HS2_D2，HS2_D3线）的卡

* 使用FATFS库（和格式卡，如果无法挂载文件系统）挂载FAT文件系统，

* 在VFS中注册FAT文件系统，从而启用C标准库和POSIX函数。


## 硬件连接

硬件连接分为 `4线SDIO`、`1线SDIO` 和 `SPI` 三种情况。

开发板使用的是 `SPI模式` 连接。

ESP32 pin     | SD card pin | SPI pin | Notes
--------------|-------------|---------|------------
GPIO14 (MTMS) | CLK         | SCK     | 10k pullup in SD mode
GPIO15 (MTDO) | CMD         | MOSI    | 10k pullup, both in SD and SPI modes
GPIO2         | D0          | MISO    | 10k pullup in SD mode, pull low to go into download mode (see Note about GPIO2 below!)
GPIO4         | D1          | N/C     | not used in 1-line SD mode; 10k pullup in 4-line SD mode
GPIO12 (MTDI) | D2          | N/C     | not used in 1-line SD mode; 10k pullup in 4-line SD mode (see Note about GPIO12 below!)
GPIO13 (MTCK) | D3          | CS      | not used in 1-line SD mode, but card's D3 pin must have a 10k pullup
N/C           | CD          |         | optional, not used in the example
N/C           | WP          |         | optional, not used in the example


## SDIO/SPI的CLK速率测试

`SDIO`模式下，`CLK`速率可任意配置，常用 `40MHz`、`20MHz`。

`SPI`模式下，因 `SD卡`在SPI模式下最大频率仅为`25MHz`，故配置的值不能超过此项。而在IDF的配置中，`SD-SPI`的默认频率为`20MHz`。


## 运行现象

* LED闪烁。

* 打开终端，ESP32上电运行后，会初始化 SD卡、FATFS文件系统，创建、重命名、写入、读取文件。终端会显示出ESP32挂载的SD卡名称。


## 如何修改例程SDIO的速率

SDIO-4线模式下，引线过长会导致通信失败。而同样的走线，1线模式或者SPI模式好很多。
修改`sdmmc_types.h`中的`SDMMC_FREQ_DEFAULT`，默认为20MHz，最大40MHz，受限于总线的速度。


## 如何修改SPI模式

```
// 要启用SPI模式，请取消注释以下行：

// #define USE_SPI_MODE
```
注意：如果使用SPI模式，还要注意是否使用了LCD，并且两个DMA通道不能相同。
尽量使用1线/4线的SDIO，占用的IO更少，速率更高。


## 如何修改SDIO-1线模式

引线过长，很容易导致4线SDIO初始化失败，可以切换到1线模式
```
// 要使用1线SD模式，请取消注释以下行：
// slot_config.width = 1;
```

## 如何修改SDIO-4线模式

默认就是SDIO-4线模式，但请将上两处都注释掉


## 关键函数

乐鑫`ESP32-FATFS`帮助文档：[ESP32-FATFS](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/storage/fatfs.html)

函数功能及更多介绍详见官网在线文档。
`FATFS`官网：[FATFS在线文档](http://www.elm-chan.org/fsw/ff/00index_e.html)


## 注意事项

* SDIO-4线模式下，引线过长会导致通信失败。而同样的走线，1线模式或者SPI模式好很多。

* 即使卡的D3线未连接至ESP32，仍必须将其上拉，否则卡将进入SPI协议模式。

* SD卡的Block为4K，而FATFS最小也需要4K的缓存，所以创建任务时要多留出一些内存，否则会反复重启。

* SD卡的SPI默认使用DMA是 DMA1，与 LCD使用的SPI的 DMA2区分开，两者不能相同。


## 如出现不能下载代码的现象

GPIO2引脚用作自举引脚，应为低电平才能进入UART下载模式。一种方法是使用跳线连接GPIO0和GPIO2，然后，大多数开发板上的自动复位电路会在进入下载模式时将GPIO2和GPIO0一起拉为低电平。所以设计PCB时尽量别用GPIO2做SD卡的引脚，建议做输出引脚，如LED输出。（编译官方sd_card后，烧录，很大概率会出现不能自动烧录的情况，就是GPIO2被拉高的原因，需要将GPIO2浮空或者手动拉低）

Connecting........_____....._____....._____....._____....._____....._____....._____

断开SD卡D0 / MISO线与GPIO2的连接，然后尝试再次上传。 阅读上面的“关于GPIO2的注意事项”。
