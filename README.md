# Easyio 开源驱动库

***

`Easyio` 是一款适配于`ESP-IDF`框架的`开源驱动库`，以支持`ESP32`的简便开发。目的是简化乐鑫`ESP-IDF`开发框架的使用难度。（真要方便的话，有现成的Arduino和Platform可以用，不过实在迫于工作要求，有的开源东西不让用，同时便于对接FAE，于是就有了 `Easyio`）

功能上，`Easyio` 已初具雏形，目前涵盖如下的驱动：
* LED、GPIO（+中断）
* 按键（队列方式，数目几无上限）、触摸按键
* ADC（8通道）、DAC（2通道）
* LEDc、PWM（+输入捕获）、PCNT（编码器计数）
* RMT红外、RMT-WS2812B-RGB灯带
* UART、RS485
* I2C_TOOLS、I2C_MPU6050、I2C_AHT20
* SPI液晶屏（支持`ST7735`、`ST7735S`、`ST7789V`、`ILI9341`、`ILI9488`、`ILI9481`、`ST7796S`、`HX8357C`8种IC，涵盖3.5寸以下的绝大多数液晶模组）
* SPI_AS5047P/TLE5012B 磁编码器。
* FT5/6xxx电容触摸屏。
* NVS、FATFS、SD_CARD（SPI模式，1/4线SDIO模式）。

<br/>

&emsp;&emsp;借助`ESP-IDF`强大的框架底层，`Easyio`在此基础上对其API进行了二次封装，并添加了许多常用外设芯片的驱动代码。做到了一行API一个外设，一段代码一个功能。完善库的同时，给每个外设都提供了测试`Demo`，每个`Demo`都提供了`README`文档，并在`.c.h`源码保留了大量注释，夸张点说注释真的要比代码多，纯小白也能用的懂。

&emsp;&emsp;目前已提供包含以上功能的32个`Demo`，未来打算继续加入jpeg解码库、OTA、LVGL、WIFI/BLE、有线以太网和一些网络通信的Demo，丰富库的应用范围。

***

# 如何使用

&emsp;&emsp;库的导入方式异常简便，直接拖入工程的 `components` 文件夹下。并包含头文件`#include "easyio.h"`，即可使用Easyio的全部功能。

&emsp;&emsp;使用教程见我的CSDN博客：
[ESP32开源驱动库Easyio的使用](https://blog.csdn.net/Mark_md/article/details/120157812?spm=1001.2014.3001.5501)

&emsp;&emsp;开发板计划也在进程中，之前做了一版偏向于HMI的设计，出于外观考量接口都留在了背面，导致留出的接口较少且指示意义不强，对开发来讲并不友好，下一版会偏向传统设计。平铺放置，但功能毫不含糊，Demo提到的外设它都会有。硬件资料会在验证无误后同步上传。

&emsp;&emsp;看都看到这儿了，还不多多star、点赞收藏。๑乛◡乛๑ 嘿嘿

***

# 清理工程，减小体积

&emsp;&emsp;`cleanBuild.bat` 为编译文件的`清理脚本`，鼠标双击运行，可删除同文件夹下所有工程的编译文件。一般一个工程的大小在几百KB，但编译后会生成100多MB的 `build` 文件，清理后可大大减少存储占用空间。每次push源码前先运行下此脚本，清理所有工程，方便同步到 `Github` 仓库。

***

# 注意

* `Easyio`建议搭配`ESP-IDF`的`V4.2`版本使用，V4.3版本与我的`Easyio`驱动库有兼容性问题。发现很多奇怪现象，不单单是编译不通过的问题，有些能编译但在硬件上运行报错，就很离谱。而这些问题在`V4.2`上则不存在。鉴于`V4.2`已经对ESP32提供了足够的支持，`V4.3`着重添加了`ESP32-S2/C3`的功能，而自己的`easyio`库暂时没有兼并`ESP32-S2/C3`的精力，于是推荐使用`V4.2`版本。

***

# 乐鑫ESP-IDF 在线帮助文档

[ESP-IDF 快速入门](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/index.html#get-started-step-by-step)

[ESP-IDF API编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/index.html)

[ESP32 硬件设计参考](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/hw-reference/index.html)
