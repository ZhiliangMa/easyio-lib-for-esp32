# 37_JPG_LCD_DMA_SD_PHOTO_ALBUM

## 例程简介

制作一个用SD卡存储照片的`电子相册`，它会读取SD卡/pic目录下的所有jpg图片，依次进行解码并显示在`320x240`分辨率的液晶屏上，并循环播放。(因为只是做一个Demo，所以没有提供操作功能，便于二次修改)

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **photo_album_task** ，制作一个用SD卡存储照片的电子相册，它会读取SD卡/pic目录下的所有jpg图片，依次进行解码并显示在`320x240`分辨率的液晶屏上，并循环播放。

(使用了之前Demo的 SD_CARD、FATFS、VFS、TJpgDec解码、LCD-SPI-DMA双缓冲加速，SD卡与LCD共用同一SPI，分时复用，以达到ESP32的最大化IO利用效率)


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
GPIO25        |         |             | RESET   |
GPIO22        |         |             | D/C     |
GPIO21        |         |             | BLK     |


## 运行现象

* LED闪烁。

* 上电后，液晶屏会先显示`Hello! TJpgDec`持续2s。之后会读取SD卡/pic目录下的所有jpg图片，依次进行解码并显示在`320x240`分辨率的液晶屏上，并循环播放。


## 关键函数

```c
// 初始化lcd使用`SPI-DMA 双缓冲环形队列`刷屏（申请DMA双缓冲内存）
void lcd_spi_dma_display_init(void);

// 刷一整个LCD屏幕（传输使用DMA环形队列加速）
void lcd_spi_dma_display_img(spi_device_handle_t spi, uint16_t ***pixels);

// 使用波动动效刷一整个LCD屏幕（传输使用DMA环形队列）并在lcd右下角显示帧率
void lcd_spi_dma_display_pretty_colors(spi_device_handle_t spi, uint16_t ***pixels);

// 申请jpg解码器、和输出到LCD的RGB565像素 需要的内存空间（此为TJpgDec解码器的预处理，仅需调用一次。重复调用可能会因为申请的空间超出ESP32可用范围，导致错误。）
esp_err_t jpg_decode_request_ram(uint16_t ***pixels, uint16_t buf_height, uint16_t buf_width);

// 解码jpg图片，输出为RGB565的像素格式
esp_err_t jpg_decode(const unsigned char *jpg_img, uint16_t ***pixels, uint8_t scale);
```

乐鑫`ESP32-SPI`帮助文档：[ESP32-SPI Master](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/peripherals/spi_master.html#)

函数功能及更多介绍详见官网在线文档。
`TJpgDec`官网：[TJpgDec在线文档](http://www.elm-chan.org/fsw/tjpgd/00index.html)

乐鑫`ESP32-FATFS`帮助文档：[ESP32-FATFS](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/storage/fatfs.html)

函数功能及更多介绍详见官网在线文档。
`FATFS`官网：[FATFS官网](http://www.elm-chan.org/fsw/ff/00index_e.html)

虚拟文件系统 (VFS)详细介绍：[ESP32-VFS虚拟文件系统组件](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/storage/vfs.html)


## 解码sd卡内.jpg的流程

解码ESP32片内的.jpg文件可以直接用指针，而解码片外的.jpg，如SD卡则过程略有不同。

**解码sd卡内.jpg图片的流程**：

先挂载SD卡、FATFS，之后打开文件，读.jpg文件，根据.jpg文件大小申请内存，再将文件读到刚刚申请的内存中。之后就跟片内.jpg的解码一样了，解码，SPI-DMA刷屏，释放内存。如此往复。

(可以预先申请好一段固定大小的内存，之后不释放，一直用于缓存.jpg图片，来节约申请和释放的时间，提高程序运行效率。但因为摸不准到底能剩多少内存，需要多次尝试)


## 对大于8.3长文件名的支持。

修改`IDF`工程配置，使`FATFS`支持`长文件名`，见博文教程：[ESP32-IDF给FATFS添加长文件名支持](https://blog.csdn.net/Mark_md/article/details/120290946?spm=1001.2014.3001.5501)


## 屏幕缓刷

使用`40/80MHz`去刷屏，会因画面切换突兀，造成断帧等不良观感。于是将SPI速率降低到`1MHz`，制造出类似缓刷的效果，图片切换平滑，观感体验更好。


## 注意事项

* SD卡不要用寨牌的，寨牌很可能挂载时就识别不到。

* Demo中，对/pic目录下的文件大小、格式没有限制。不会因为文件大小超出可用内存，而导致申请内存失败，进而重启。也不会因为图片格式错误，解码失败而导致重启。

* 在遇到/pic目录下的图片大小、格式超出可处理范围时，会进入异常处理，直接跳过此文件，继续去处理下一个。以免导致重启影响到任务正常运行。

* 解码错误的原因，大概率是因为引入了QQ截图的.jpg图片。在此为了任务考虑，暂不做处理。(可将QQ截图粘贴到`画图`中，再另存为.jpg文件，这样就能被正常解码)

* 刷屏过后，在导入下一副照片前，记得释放存放.jpg文件的内存。free(jpg_img);

* 测试过程中，部分模组的ST7789，会在80MHz频率下刷屏，会造成画面镜像。如有此现象建议缩短排线距离，或者降低速率。

* ESP32在同时使用多种模组时，如本节的 SD卡、SPI-DMA、LCD、jpg解码。可申请的内存较少，仅有40KB左右，也就是说仅能解码40KB以下的图片。对于更大的图片解码并显示，建议扩展PSRAM。
