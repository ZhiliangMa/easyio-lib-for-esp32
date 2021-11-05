# 39_WIFI_softAP

## 例程简介

此例程照搬ESP-IDF的 wifi -> getting_started -> softAP。

修改了`Kconfig.projbuild`，使得可以在`menuconfig`配置连接的 WIFI热点名称、WIFI密码。

`menuconfig`中可配置 连接的WIFI热点名称、WIFI密码、WIFI网络通道、连接STA的最大连接数。

ESP32上电运行后，会开启WIFI，设置为`AP模式`，用预设的WIFI热点名称、WIFI密码来创建热点。连接成功后，ESP32会通过log告知创建的热点信息。

详细介绍可查阅 ESP-IDF目录下的的 wifi -> getting_started -> softAP 例程。

由于softAP几乎不单独使用，且也不会像本例程main中的一长串。故此源码仅作演示，后面有更方便的api来创建热点。

更多API介绍，详见乐鑫的在线文档：[ESP32 Wi-Fi 驱动程序](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/wifi.html)

# 运行现象

编译源码前，需要配置`menuconfig`中的 "WiFi SSID" 和 "WiFi Password"，代码运行后会按照此信息来创建热点。

如编译正常，运行后会输出如下内容：

```
I (0) cpu_start: App cpu up.
I (427) heap_init: Initializing. RAM available for dynamic allocation:
I (433) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (439) heap_init: At 3FFB7800 len 00028800 (162 KiB): DRAM
I (446) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (452) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (458) heap_init: At 400955FC len 0000AA04 (42 KiB): IRAM
I (465) cpu_start: Pro cpu start user code
I (483) spi_flash: detected chip: generic
I (484) spi_flash: flash io: dio
I (484) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (493) gpio: GPIO[33]| InputEn: 1| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (533) wifi softAP: ESP_WIFI_MODE_AP
I (553) wifi:wifi driver task: 3ffc0b44, prio:23, stack:6656, core=0
I (553) system_api: Base MAC address is not set
I (553) system_api: read default base MAC address from EFUSE
I (573) wifi:wifi firmware version: bb6888c
I (573) wifi:wifi certification version: v7.0
I (573) wifi:config NVS flash: enabled
I (573) wifi:config nano formating: disabled
I (583) wifi:Init data frame dynamic rx buffer num: 32
I (583) wifi:Init management frame dynamic rx buffer num: 32
I (593) wifi:Init management short buffer num: 32
I (593) wifi:Init dynamic tx buffer num: 32
I (603) wifi:Init static rx buffer size: 1600
I (603) wifi:Init static rx buffer num: 10
I (603) wifi:Init dynamic rx buffer num: 32
I (613) wifi_init: rx ba win: 6
I (613) wifi_init: tcpip mbox: 32
I (623) wifi_init: udp mbox: 6
I (623) wifi_init: tcp mbox: 6
I (623) wifi_init: tcp tx win: 5744
I (633) wifi_init: tcp rx win: 5744
I (633) wifi_init: tcp mss: 1440
I (643) wifi_init: WiFi IRAM OP enabled
I (643) wifi_init: WiFi RX IRAM OP enabled
I (653) phy_init: phy_version 4660,0162888,Dec 23 2020
I (753) wifi:mode : softAP (c4:dd:57:b8:fa:15)
I (763) wifi:Total power save buffer number: 16
I (763) wifi:Init max length of beacon: 752/752
I (763) wifi:Init max length of beacon: 752/752
I (763) wifi softAP: wifi_init_softap finished. SSID:easyio password:a123456789 channel:1
```


## 注意事项

* 连接ESP32的WIFI热点，需要使用2.4GHz的频段。
