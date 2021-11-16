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

LCD型号，优先推荐 `ILI9341` 或 `ST7789V`，可完美支持ESP32的SPI-DMA刷屏。优化了背光上电顺序，使得屏幕刚显示后的一瞬间不会出现花屏。

开发板套餐中的液晶模组，为单点电容触摸屏，2寸320*240。IC型号为 `ST7789V` + `FT6236U`。【SPI速率最高可达80MHz】

开发板还提供了一行接口J5，用于扩展LCD模组，可兼容大部分`LCD/OLED 模组`的管脚顺序。【需要注意的是，受模组走线的影响，此位的SPI速率一般最大仅为40MHz】


## 硬件连接

|                  | CLK    | MOSI   | DC     | BLK    | RST    | CS     |
| ---------------- | ------ | ------ | ------ | ------ | ------ | ------ |
| ESP32 SPI Master | GPIO18 | GPIO23 | GPIO22 | GPIO21 | GPIO25 | GPIO5  |
| lcd              | CLK    | MOSI   | DC     | BLK    | RST    | CS     |

- `ESP32-IOT-DEV` 开发板不仅支持`J3`位的`22P-FPC排线液晶屏-ST7789V`，还可在`J5`的9P排母位置，插接 `LCD/OLED` 模组，以支持更多的屏幕型号和模组。

- `ESP32-IOT-DEV` 开发板对`J3`位的`22P-FPC排线液晶屏-ST7789V`，支持最大 `80MHz` 的SPI驱动速率。

- 若开发板使用`22P-FPC排线液晶模组`，在 `80MHz` 的SPI速率下出现轻微花屏，可在TF卡位(`J6`)插接上TF卡。
【实测有效，绝大多数的花屏都会消除。即使是差的情况，仍会有一些斑点，但不会影响液晶屏的正常驱动。连续刷屏异常反倒会消失，绝大多数是刷一帧时才会出现上述花屏现象】

- `80MHz` 对`J5`位的LCD模块无用，`J5`位插接的模块最大 `40MHz`。


## 如何修改工程，使其匹配不同的LCD驱动IC ?

本Demo通过对`IDF`的`LCD`例程进行扩展，使得支持的LCD驱动IC型号，从原来的`ili9341`、`st7789`两种，扩展到 ST7735、ST7735S、ST7789V、ILI9341、ILI9488、ILI9481、ST7796S、HX8357C 如此之多。

得益于乐鑫完善的LCD驱动框架，可以很方便的导入不同屏幕的配置文件。`easyio`将整个流程封包，添加了多余屏幕的支持，并留出两个配置LCD的接口`spi_master_init`、`spi_lcd_init`，和一个配置文件 `spi_lcd.h`。

`easyio` 匹配不同驱动IC的`LCD`的步骤：

1. 使用 `spi_master_init` 配置LCD的 `SPI通道` 和 `SCL(SPI-SCK)`、`SDA(SPI-MOSI)` 引脚。(建议使用`IO_MUX`默认的引脚，以支持最大80MHz的通信速率。非`IO_MUX`默认引脚最高仅能27MHz)

2. 使用 `spi_lcd_init` 配置SPI与LCD的 `通信速率` 和 `CS` 引脚。

3. 在 `spi_lcd.h` 中配置 `LCD驱动IC型号`、`LCD像素分辨率`、`屏幕显示方向`、`D/C引脚GPIO`、`RESET引脚GPIO`、`BLK引脚GPIO`。

之后就可调用 `"simple_gui.h"` 中丰富的接口，在lcd上显示 点、线、矩形、圆、数字、字符串、位图图片。

驱动中已对上电顺序进行优化，初始化LCD的过程中不会产生异常闪屏和花屏，开启显示后默认显示纯黑。(但因为此时背光已开启，这里的黑并不是完全黑，还是有一部分光会从lcd漏出来)


## IO_MUX 输出方式下 CLK的速率测试

通过更改 `spi_lcd_init(SPI3_HOST, 40*1000*1000, LCD_SPI3_DEF_PIN_NUM_CS0);` 中的时钟速率，并通过示波器捕获观察实际频率。

`SPI`的GPIO使用 `IO_MUX` 的默认端口时，最大频率可设置为 `80MHz`，示波器实际测得频率为 `80MHz`。

配置速率为 `60MHz`，示波器实际测得频率则为 `40MHz`。

配置速率为 `40MHz`，示波器实际测得频率则为 `40MHz`。

通过对几种不同驱动IC的屏幕模组测试，`ST7789V` 可完美运行在80MHz。ILI9488、HX8357C 为 40MHz（80MHz可能因线路会花屏）。ILI9481 仅为 10MHz。结果可能有差异，仅供参考。(大多数IC的SPI通信跑在40MHz都没问题，很稳定也不会花屏)


## 运行现象

* LED闪烁。

* lcd液晶屏刷屏，显示 点、线、矩形、圆、数字、字符串、位图图片。字符支持`1206`、`1608`两种字体。

*  修改 `spi_lcd.h` 中的 `CONFIG_LCD_TYPE_ST7789V`，将其改为其他注释的值，即可修改`LCD的驱动IC型号`。

* 修改 `spi_lcd.h` 中的 `LCD_PIXEL_SIZE_240_320`，将其改为其他注释的值，即可修改`分辨率`。

* 修改 `spi_lcd.h` 中的 `LCD_DIR`、`LCD_INVERT`、`LCD_MIRROR`，即可对屏幕的 `显示方向`、`镜像方式` 进行更改。

* `ESP32-IOT-DEV` 开发板不仅支持`J3`位的`22P-FPC排线液晶屏-ST7789V`，还可在`J5`的9P排母位置，插接 `OLED/LCD` 模组，以支持更多的屏幕型号和模组。


## 学习内容

1. SPI 总线时序。

2. SPI写LCD的GRAM时序。

3. 图片取模的方法。

4. 在lcd上显示 点、线、矩形、圆、数字、字符串、位图图片。

扩展了解：[ESP32驱动LCD液晶屏的选型，SPI写LCD的GRAM时序](https://blog.csdn.net/Mark_md/article/details/115620490?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163065547716780366549101%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163065547716780366549101&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29-1-115620490.pc_v2_rank_blog_default&utm_term=lcd&spm=1018.2226.3001.4450)


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

* `ESP32-IOT-DEV` 开发板对`J3`位的`22P-FPC排线液晶屏-ST7789V`，支持最大 `80MHz` 的SPI驱动速率。

* 而`J5`位虽然可以插接众多`LCD/OLED模组`，但因受LCD模组走线影响，一般最大速率仅为 `40MHz`。

* 若开发板使用`22P-FPC排线液晶模组`，在 `80MHz` 的SPI速率下出现轻微花屏，可在TF卡位(`J6`)插接上TF卡。
【实测有效，绝大多数的花屏都会消除。即使是差的情况，仍会有一些斑点，但不会影响液晶屏的正常驱动。连续刷屏异常反倒会消失，绝大多数是刷一帧时才会出现上述花屏现象】

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

* TB上买到的`40P`液晶模组。在`SPI模式`下，有两个端口的功能不一致。部分型号的Pin10、Pin11两个端口的功能正好相反，在硬件设计时需要注意兼容性：

ILI9488、ILI9481、ST7796S、HX8357C，这些分辨率为`320x480`的液晶模组，其Pin10、Pin11 的功能为 D/C、SCK。

ST7735、ST7735S、ST7789V、ILI9341，这些分辨率为`240x320`的液晶模组，其Pin10、Pin11 的功能为 SCK、D/C。

* 后续更新：

SD卡、FATFS文件系统、VFS、SD卡与LCD共用一条SPI。

jpg/jpeg解码。

`SPI-DMA双缓存环形队列`，刷屏，有图像缓存的情况下，ESP32可以很高效的利用SPI的带宽。`320x240`分辨率，RGB565像素格式，以理论80MHz计算，满带宽下可以65.1FPS。而实际在有图片缓存的情况下，`SPI-DMA双缓存环形队列`可以达到53FPS。40MHz的带宽利用率更高一些，可达30FPS。（但ESP32的浮点运算好像不怎么样，运行乐鑫的波动动效Demo，因为浮点运算的拖累，导致上一帧DMA已经发送完了，可下一帧的图像还在计算中，还是要等计算完成才能发送下一帧，于是被拖累了显示成绩，只有18FPS）（见Demo-`36_JPG_LCD_DMA_EFFECT`）

GUI-LVGL-Demo。


## 乐鑫LCD驱动框架解读

以下比较啰嗦，仅作经验之谈。

因为我是在搞完easyio后，才对LVGL官方的ESP32例程进行了解。`lv_port_esp32`支持了更多的LCD和触摸驱动IC型号、并且把屏幕和触摸的驱动部分都做好了。clone工程后，只需要menuconfig配置下驱动IC型号、时钟速度等基本硬件信息，就能够编译下载了。

在LVGL的测试过程中，发现`lv_port_esp32`对`st7789`的支持并不好，可能是自己买到的驱动IC型号为`st7789V`的缘故，显示时严重花屏，而相同配置下，选择`ili9341`编译输出，则能正常显示GUI界面。

因`st7789`与`ili9341`的刷GRAM的方式相同，唯一不同的是上电后的寄存器配置。

由于有前面写easyio时积累的LCD配置经验，于是着手对LVGL的`st7789`驱动进行更改。更改的时候发现，LVGL官方Demo的LCD驱动，也是沿用乐鑫`lcd`例程的那一套。这一下就方便多了，把 `st_7789V_init_cmds` 直接替换掉，LCD就可以正确显示了。

乐鑫`lcd`例程对LCD驱动IC的配置文件写入，`easyio`将其封装为了`lcd_ic_init`，关键代码也只有下面这么几句：
```c
    // 将spi_lcd.h 中的参数写入LCD驱动IC
    while (lcd_init_cmds[cmd].databytes!=0xff) {
        lcd_cmd(spi, lcd_init_cmds[cmd].cmd);
        lcd_data(spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes&0x1F);
        if (lcd_init_cmds[cmd].databytes&0x80) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
        cmd++;
    }
```

驱动IC的每个寄存器配置项，被拆分为成了`结构体` - `lcd_init_cmd_t`。一个IC的所有配置文件，被整合成`结构体数组`，位于 `spi_lcd.h`文件中。

上面的代码通过判断结构体中的`databytes`，来向LCD写入配置文件。

```c
/*
 LCD初始化需要的一堆命令/参数值。 它们存储在此结构中。
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;
```

不同内容的`databytes`有不同的含义，其定义为：

* 如果`databytes`大于1，且不为`0xff`，则向LCD的寄存器地址写入指定字节数的数据。

向`0x3A`写入`1`个字节`0x05`；向`0xB2`写入`5`个字节`0x0c, 0x0c, 0x00, 0x33, 0x33`。
```c
    /* Interface Pixel Format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x05}, 1},
    /* Porch Setting */
    {0xB2, {0x0c, 0x0c, 0x00, 0x33, 0x33}, 5},
```

* 如果`databytes`为`0x80`，则发送`单字节命令`，发送完成后会延时10ms继续发送下一帧命令。单字节命令通常有`0x11`退出休眠、`0x29`开启显示、`0x2C`写GRAM。
```c
    /* Sleep Out */
    {0x11, {0}, 0x80},
    /* Display On */
    {0x29, {0}, 0x80},
```
* 而一旦`databytes`为`0xff`，则意味着LCD配置文件的写入完成，退出循环。之后就可以操作GRAM和背光，让LCD显示出图像。这个会放在配置文件的最末尾。
```c
    {0, {0}, 0xff}
```

以上的配置文件，因不同型号的驱动IC参数不一致，需要每个型号一组单独的配置文件。

而刷屏则简单很多，流程也都相同。

用 `LCD_SetCursor` 或 `LCD_SetWindows` 设置`光标位置`、`写入GRAM的范围`。之后将RGB像素内容写入GRAM，图像就会在液晶屏上被显示出来。
