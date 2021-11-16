# 35x_JPG_LCD_DMA_time

## 简介

本例程以`35_JPG_LCD_DMA`为蓝本，修改ESP32的运行频率为240NHz，对比`TJPG`解码耗时。(35_JPG_LCD_DMA，ESP32运行于160MHz)


## 运行现象

* LED闪烁。

* 上电后，液晶屏会先显示`Hello! TJpgDec`持续2s。之后会以2s为间隔，在两幅图片间来回切换。

经测试：

- ESP32运行于160MHz，解码Demo中的两个图片分别用时136ms和131ms。【如不对SDK的配置进行修改，默认运行频率即为160MHz】

- ESP32运行于240MHz，解码Demo中的两个图片分别用时91ms和88ms。耗时基本为160MHz的2/3，合理。


## 如何修改ESP32运行频率

ESP32的默认运行频率为`CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ`，可在menuconfig中修改。

ESP-IDF默认的运行频率为 160MHz。

详细说明可见[ESP-IDF 电源管理](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/system/power_management.html)
