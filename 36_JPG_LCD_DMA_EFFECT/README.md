# 36_JPG_LCD_DMA_EFFECT

## 例程简介

jpg解码 + SPI-DMA双缓冲环形队列刷屏 + 波动动效。

.jpg图片源存储在ESP32 Flash内，与代码一同编译后使用。

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **jpg_lcd_spi_dma_effect_task** ，初始化 SPI、LCD、jpg解码器，以`SPI-DMA 双缓冲环形队列`的方式去刷液晶屏，连续显示一张图片的波动动效。


`tjpgd.c.h`： jpg解码器`TJpgDec`的源码。

`jpg_decode.c.h`： 为jpg解码的流程。参照了lcd例程的`decode_image.c.h`，并在此上进行的优化，可进行重复解码（`decode_image.c.h`在解码后马上就释放了内存）

`lcd_spi_dma.c.h`： 为`SPI-DMA 双缓冲环形队列`刷液晶屏。其中`void lcd_spi_dma_display_pretty_colors(spi_device_handle_t spi, uint16_t ***pixels);`为以DMA的方式用波动动效刷屏。


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


## 运行现象

* LED闪烁。

* 上电后，液晶屏会先显示`Hello! TJpgDec`持续2s。之后会以波动动效刷一张图片，并在右下角显示刷屏的帧率。

* 注释掉波动动效中-进行正弦运算的代码，重新编译、下载，观察刷屏帧率。

* 更改SPI的SCK速率，观察刷屏帧率。


## 测算SPI-DMA 双缓冲环形队列的刷屏帧率

1. 先将`lcd_spi_dma.c`中的这一段代码注释：`static void pretty_effect_calc_lines()` 函数内的 `if (frame!=prev_frame) { }`。

注释后，省去了动效的正弦运算，持续刷新静态图片。可以排除掉`计算耗时`对测试刷屏速率的影响。

重新编译、下载。测试在`320x240`的屏幕，以80MHz的SPI速率驱动lcd，平均在`53.5FPS`。(40MHz为`30.2FPS`)

最高理论值FPS：80000000 / 320x240x2x8 = `65.1`

实际值与估算值相差并不是很多，因为要考虑到lcd驱动ic需要软件回调控制D/C脚，以及IDF框架自身的性能占用，这些相差是正常的，在可控范围内。

`SPI-DMA 双缓冲环形队列`的刷屏方式确实能极大提高SPI总线通信效率。

2. 再将刚刚的注释还原，使图片增加动效。

此时更改SPI速率，发现80MHz和40MHz的实际的FPS降为`18.5`和`18.1`，因为这个帧率的带宽远小于40MHz-SPI的带宽，所以无论设置为40MHz还是80MHz，帧率几乎都是一样的。

这种情况，SPI-DMA的带宽已经不是瓶颈，真正影响刷屏速率的是`计算耗时`。


## 如何更改波动动效的图片

波动动效由MCU处理，只需要输入一张静态图片，即可显示出波动动效。

片内导入图片的方法，与上一节Demo一致。但因为动效的上下左右各有8个像素的边框，所以要求输入图片的高宽，要比LCD的分辨率大16。（例如`320x240`的屏幕，输入动效的图片的分辨率要为`336x256`）


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


## 注意事项

* `get_bgnd_pixel` 和 `get_bgnd_pixel_offset` 功能不一样，后者有像素偏移。如果`pretty_effect_calc_lines`内不小心使用了`get_bgnd_pixel`，会因为指针过界，导致运行反复重启。
