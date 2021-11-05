# 35_JPG_LCD_DMA

## 例程简介

之前`Demo`中的图片使用`位图取模`的存储方式，使用起来有诸多不便：需要单独取模、占用`Flash`巨大、没有文件格式不利于大批量图片的存储。生活中，图片最常用的格式是.jpg，jpg图片是一种有损压缩的格式，虽然会损失一些局部细节，但所需存储空间大为较少。.jpg的显示需要`jpg解码器`，MCU使用较多的`jpg解码器`有的`TJpgDec`和`libjpeg`。

其中`TJpgDec`体积小巧，虽然解码速度没有`libjpeg`快，但RAM仅占用3KB大小，且方便移植。之前使用的`FATFS`也是出自同一作者。[TJpgDec官网](http://www.elm-chan.org/fsw/tjpgd/00index.html)

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **jpg_lcd_spi_dma_task** ，初始化 SPI总线、LCD、jpg解码器，并以`SPI-DMA 双缓冲环形队列`的方式去刷液晶屏，让lcd来回显示两张图片。


`tjpgd.c.h`： jpg解码器`TJpgDec`的源码。

`jpg_decode.c.h`： 为jpg解码的流程。参照了lcd例程的`decode_image.c.h`，并在此上进行的优化，可进行重复解码（内存一直被占用，而`decode_image.c.h`在解码后马上就释放了内存）

`lcd_spi_dma.c.h`： 为`SPI-DMA 双缓冲环形队列`刷液晶屏。`easyio`已将其封装好，只需要调用`lcd_spi_dma_display_init();`和`lcd_spi_dma_display_img(spi_device_handle_t spi, uint16_t ***pixels);`即可使用完整功能。


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

* 上电后，液晶屏会先显示`Hello! TJpgDec`持续2s。之后会以2s为间隔，在两幅图片间来回切换。


## SPI-DMA 双缓冲环形队列的运行机制

`SPI-DMA 双缓冲环形队列`刷屏涉及到的内容较多，完整理解起来需要些时间。所幸`easyio`已将其封装好，只需要调用`lcd_spi_dma_display_init();`和`lcd_spi_dma_display_img(spi_device_handle_t spi, uint16_t ***pixels);`即可使用完整功能。

前几节的SPI刷液晶屏，虽然也用到了DMA，但都是通过画点的方式来完成绘图。传输效率低下。本`Demo`参照了`IDF`中的`lcd`Demo，使用`SPI-DMA 双缓冲环形队列`的方式去刷屏，极大提高了刷屏帧率。SPI通信部分完全交由DMA控制，DMA发送时会占用其中一块DMA内存，在`MCU`等待`DMA发送完成`前，会进行下一帧显示图片的像素格式转换，并将转换好的像素放入DMA的另一块内存。待DMA发送完成后立马将刚刚处理好的内存交给下一帧SPI-DMA通信，如此往复。最大程度上提高了MCU资源的利用率。(在`spi_lcd.h`中设置，默认会以16行屏幕像素为一次分割，来进行一次`SPI-DMA`的传输`队列`。每个队列只发送一行的像素，包括设置坐标、写GRAM，每个队列都是一次`SPI-DMA`通信。以此来达到减小DMA内存占用的目的，并不是一次DMA刷完整个屏幕。这种方式组成的`环形DMA`，DMA内存只需要2块`PARALLEL_LINES*lcddev.width*sizeof(uint16_t)`大小)


## 如何将.jpg图片导入工程并调用

本Demo的`jpg`图片，直接以二进制文件的方式编译在`ESP32`的`Flash`中。没有使用文件系统。

**导入.jpg图片步骤**：

1. 获取`jpg`图片：如有下载好的`jpg`图片可直接使用。但如果是使用QQ截屏，就需要先粘贴到`画图`，再`另存为`-`JPEG文件`。(直接使用QQ截屏的`jpg`会解码失败，不明原因)

2. 之后将`.jpg`图片复制到工程`\components\easyio_lib\pic`下的文件夹。

3. 再修改`\components\easyio_lib\CMakeLists.txt`文件，插入如`EMBED_FILES "pic/image_esp32mcu.jpg" "pic/image_esp32wifi.jpg"`的jpg图片路径。

4. 代码中输入图片时，需要用下面的方式去引用`jpg`文件的起始地址：

```c
// 引用二进制化的jpeg文件的首尾地址
extern const uint8_t image_esp32mcu_jpg_start[] asm("_binary_image_esp32mcu_jpg_start");
extern const uint8_t image_esp32mcu_jpg_end[] asm("_binary_image_esp32mcu_jpg_end");

extern const uint8_t image_esp32wifi_jpg_start[] asm("_binary_image_esp32wifi_jpg_start");
extern const uint8_t image_esp32wifi_jpg_end[] asm("_binary_image_esp32wifi_jpg_end");
```

其中asm()中的内容，会根据引入工程的`jpg文件名称`而变化，需要手动修改。其详细地址可在编译后全局搜索到。


## SPI-DMA 双缓冲环形队列的刷屏速度

刷屏帧率测试会在下一个Demo演示。并会演示`IDF`的`lcd`例程自带的一个波纹动效。

（剧透：`320x240`的屏幕，以80MHz的SPI速率驱动lcd，平均在`53.5FPS`。【40MHz为`30.2FPS`】）


## 关键函数

```c
// 初始化lcd使用`SPI-DMA 双缓冲环形队列`刷屏（申请DMA双缓冲内存）
void lcd_spi_dma_display_init(void);

// 刷一整个LCD屏幕（传输使用DMA环形队列加速）
void lcd_spi_dma_display_img(spi_device_handle_t spi, uint16_t ***pixels);

// 申请jpg解码器、和输出到LCD的RGB565像素 需要的内存空间（此为TJpgDec解码器的预处理，仅需调用一次。重复调用可能会因为申请的空间超出ESP32可用范围，导致错误。）
esp_err_t jpg_decode_request_ram(uint16_t ***pixels, uint16_t buf_height, uint16_t buf_width);

// 解码jpg图片，输出为RGB565的像素格式
esp_err_t jpg_decode(const unsigned char *jpg_img, uint16_t ***pixels, uint8_t scale);
```

乐鑫`ESP32-SPI`帮助文档：[ESP32-SPI Master](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/peripherals/spi_master.html#)

函数功能及更多介绍详见官网在线文档。
`TJpgDec`官网：[TJpgDec在线文档](http://www.elm-chan.org/fsw/tjpgd/00index.html)


## 注意事项

* 虽然将jpg解码过程进行了优化，使得可在只占用3KB+一屏幕像素内存的情况下，能进行重复解码。但如果想将多张解码后的图片同时放入RAM，考虑到ESP32的RAM大小，当显示图片的分辨率大于等于`320x240`时，就不要重复调用`jpg_decode_request_ram`了，会导致申请内存失败(320x240-RGB565图片占用RAM：320x240x2 = 153600Bytes = 150KB。虽然ESP32有512KB的SRAM，但运行IDF框架后，可用内存仅为200KB左右)。

* ESP32 如何查看芯片内存（例如：DRAM、IRAM、rodata）使用情况？：[ESP-FAQ](https://docs.espressif.com/projects/espressif-esp-faq/zh_CN/latest/software-framework/storage.html#esp32-dramiramrodata)

* ESP32内存模型：[ESP32内存模型](https://blog.espressif.com/esp32-programmers-memory-model-259444d89387)

* ESP32应用程序的内存布局：[ESP32内存布局](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/memory-types.html#iram-ram)

* [ESP32堆内存分配](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/system/mem_alloc.html)

* [ESP32应用程序启动流程](https://esp-idf-zh.readthedocs.io/zh_CN/latest/api-guides/general-notes.html)

***

* `SPI-DMA 双缓冲环形队列`暂不支持`ili94xx`型号的显示。因 `ili94xx` 在`SPI`模式下，只能使用 3Bytes-RGB666 的像素格式，显示效果一般，且3字节的方式对SPI带宽及内存浪费严重。所以本Demo的`SPI-DMA 双缓冲环形队列`刷屏，目前只对2Bytes-RGB565做了适配。

* `HX8357C`因为像素偏移导致图片错位，SPI-DMA加速目前仅对 `ili9341`、`st7789` 完美支持。

* 如果用QQ截图取图，不能直接使用QQ截图保存的.jpg，会导致解码失败。需要先粘贴到`画图`，然后另存为.jpg。

* 为匹配液晶屏的RGB565像素格式，`tjpgd.h` 中的`JD_FORMAT`应设置为1，设置为1:RGB565 (1 WORD/pix)。为0:RGB888 (3 BYTE/pix)会出现乱码。

* 更改LCD的SPI-SCK时钟速率，10MHz、20MHz刷屏时会感觉到轻微拖影，而设置为40MHz、80MHz，基本感觉不到，瞬切。(80MHz对线路要求严格，有可能因线路问题导致花屏。)

* 与`TjpgDec`相似的jpg解码库还有`libjpeg`，解码速度略快、功能更多，但所需的RAM和Flash空间也更大些。

* 刷屏帧率测试会在下一个Demo演示。并会演示`IDF`的`lcd`例程自带的一个波纹动效。
