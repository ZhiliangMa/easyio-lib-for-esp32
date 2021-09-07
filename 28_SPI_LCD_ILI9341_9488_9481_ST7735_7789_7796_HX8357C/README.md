# 28_SPI_LCD_ILI9341_9488_9481_ST7735_7789_7796_HX8357C

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **spi_lcd_task** ，初始化 SPI3-LCD，并显示图形。

使用 `spi_config.c.h` 驱动模块，对ESP32的 `SPI` 进行配置。

使用 `spi_lcd.c.h` 驱动挂载在 `SPI` 总线上的 `lcd`液晶屏设备。

并提供了一个小型的GUI库 `simple_gui.c.h`，与stm32正点原子的lcd例程类似，可以在`lcd`上显示 点、线、矩形、圆、数字、字符串、位图图片。字符支持`1206`、`1608`两种字体。

支持驱动IC为 ST7735、ST7735S、ST7789V、ILI9341、ILI9488、ILI9481、ST7796S、HX8357C，几乎涵盖了SPI小屏的大部分型号。
（如更改驱动IC型号，请到 `spi_lcd.h` 中修改。可设置参数有 `LCD驱动IC型号`、`LCD屏幕像素分辨率`、`屏幕的安装方向`，设置`屏幕的安装方向`仅对ILI的大部分型号有用）


## 硬件连接

|                  | CLK    | MOSI   | DC     | BLK    | RST    | CS     |
| ---------------- | ------ | ------ | ------ | ------ | ------ | ------ |
| ESP32 SPI Master | GPIO18 | GPIO23 | GPIO22 | GPIO21 | EN     | GPIO5  |
| lcd              | CLK    | MOSI   | DC     | BLK    | RST    | CS     |


## 运行现象

* LED闪烁。

* lcd液晶屏刷屏，显示 点、线、矩形、圆、数字、字符串、位图图片。字符支持`1206`、`1608`两种字体。


## 学习内容

1. SPI 总线时序。

2. SPI写LCD的GRAM时序。

3. 图片取模的方法。

4. 在lcd上显示 点、线、矩形、圆、数字、字符串、位图图片。

ESP32驱动LCD液晶屏选型，SPI写LCD的GRAM时序：https://blog.csdn.net/Mark_md/article/details/115620490?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163065547716780366549101%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163065547716780366549101&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29-1-115620490.pc_v2_rank_blog_default&utm_term=lcd&spm=1018.2226.3001.4450


## 关键函数

```c
// 配置SPIx主机模式，配置DMA通道、DMA字节大小，及 MISO、MOSI、CLK的引脚。
void spi_master_init(spi_host_device_t host_id, int dma_chan, uint32_t max_tran_size, gpio_num_t miso_io_num, gpio_num_t mosi_io_num, gpio_num_t clk_io_num);

// 以SPI方式驱动LCD初始化函数
void spi_lcd_init(spi_host_device_t host_id, uint32_t clk_speed, gpio_num_t cs_io_num);

// GUI清屏函数
void LCD_Clear(uint16_t color);

// GUI画线函数
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

// GUI画点函数
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);

// GUI画空心矩形函数
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

// GUI画实心矩形函数
void LCD_DrawFillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

// GUI画空心圆
void LCD_DrawCircle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color);

// GUI画角度线
void LCD_DrawAngleLine(uint16_t x,uint16_t y,uint16_t angle,uint16_t r,uint16_t color);

// GUI显示单个字符
void LCD_ShowChar(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,uint8_t ch,uint8_t size,uint8_t mode);

// GUI显示字符串
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,char *p,uint8_t size,uint8_t mode);

// GUI显示整形数字（空余位有空格显示）
void LCD_ShowNum(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);

// GUI显示RGB565（16Bit）的图片
void LCD_ShowPicture_16b(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t *p);
```


## 注意事项

* ESP32共有4个SPI控制器：SPI0、SPI1、SPI2、SPI3。3组SPI接口：SPI、HSPI、VSPI。其中对用户开放使用的只有 SPI1~3这三个。

* ESP32的普通GPIO最大只能30MHz，而IOMUX默认的SPI，CLK最大可以设置到80MHz。所以为了提高刷屏速度，尽量使用硬件的IOMUX端口。

* ESP32的SPI默认使用DMA，在传输长度较长的数据时可以明显提高效率。

* ESP32的SPI驱动LCD比较方便的一点是：有SPI预传输回调。（在SPI传输即将开始前调用的函数），用来处理 D/C 线的电平，控制接下来发送的内容是 指令/数据。

* 乐鑫提供的`lcd`-Demo的驱动框架非常好：默认使用SPI-DMA、有SPI预传输回调（用来自动处理D/C线）、队列实现寄存器的配置（在支持众多驱动IC型号的同时，也能保持较少的代码量）。
在后面的jpeg解码例程中，还实现了DMA的双缓冲，使得`SPI-DMA刷屏` 和 `处理GRAM像素`互不影响，极大提高了刷屏的速度。总之`ESP-IDF`的lcd例程十分值得一看。

* easyio对`lcd驱动IC型号`加以扩展，将支持的驱动IC，从 ST7735、ILI9341，扩展到 ST7735、ST7735S、ST7789V、ILI9341、ILI9488、ILI9481、ST7796S、HX8357C，几乎涵盖了SPI小屏的大部分型号。

* 设置`屏幕的安装方向`仅对ILI的大部分型号有用。

* 如有花屏等现象，建议降低SPI速率重试。（128*128分辨率的st7735屏幕，SPI速率高于30MHz容易出现这种问题）

* ili94xx 这些IC，SPI模式仅能使用 `RGB666`-18Bit/像素，分3字节传输。而不能使用16Bit/像素，分2字节传输。会使得刷屏事件变长。

* 慎用 ili9481，其最大CLK只能16MHz。且IC已停产，谨慎使用此型号。

* TB上买到的40P - ILI94xx和ILI93xx模组的屏幕接口。不是完全一致的，SPI模式下，有两条相邻的数据线IO是反的。需要兼容性请在硬件设计时注意。

* 待更新：
jpg/jpeg解码
FATFS文件系统，从sd卡读取jpg/jpeg文件
增加LVGL-GUI
