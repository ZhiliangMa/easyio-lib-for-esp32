# 38_WIFI_station

## 例程简介

此例程照搬ESP-IDF的 wifi -> getting_started -> station。

修改了`Kconfig.projbuild`，使得可以在`menuconfig`配置连接的 WIFI热点名称、WIFI密码。

`menuconfig`中可配置 连接的WIFI热点名称、WIFI密码、重试连接次数。

ESP32上电运行后，会开启WIFI，设置为STA模式，用预设的WIFI热点名称、WIFI密码来连接热点。连接成功后，ESP32会通过log告知获得的IP、子网掩码、网关等信息。

详细介绍可查阅 ESP-IDF目录下的的 wifi -> getting_started -> station 例程。

由于station几乎不单独使用，且也不会像本例程main中的一长串。故此源码仅作演示，后面有更方便的api来接热点。

更多API介绍，详见乐鑫的在线文档：[ESP32 Wi-Fi 驱动程序](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/wifi.html)

# 运行现象

编译源码前，需要配置`menuconfig`中的 "WiFi SSID" 和 "WiFi Password"，与连接的WIFI热点的名称和密码一致。

如编译正常，运行后会输出如下内容：

```
I (0) cpu_start: App cpu up.
I (427) heap_init: Initializing. RAM available for dynamic allocation:
I (434) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (440) heap_init: At 3FFB7808 len 000287F8 (161 KiB): DRAM
I (446) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (452) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (459) heap_init: At 400955FC len 0000AA04 (42 KiB): IRAM
I (465) cpu_start: Pro cpu start user code
I (483) spi_flash: detected chip: generic
I (484) spi_flash: flash io: dio
I (484) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (493) gpio: GPIO[33]| InputEn: 1| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (533) wifi station: ESP_WIFI_MODE_STA
I (553) wifi:wifi driver task: 3ffc0b78, prio:23, stack:6656, core=0
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
I (753) wifi:mode : sta (c4:dd:57:b8:fa:14)
I (763) wifi station: wifi_init_sta finished.
I (763) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,0>, prof:1
I (773) wifi:state: init -> auth (b0)
I (783) wifi:state: auth -> assoc (0)
I (783) wifi:state: assoc -> run (10)
I (803) wifi:connected with CMCC-vVSQ, aid = 4, channel 1, BW20, bssid = 2c:43:be:1c:fb:ff
I (803) wifi:security: WPA2-PSK, phy: bgn, rssi: -80
I (813) wifi:pm start, type: 1

I (813) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (2033) esp_netif_handlers: sta ip: 192.168.1.7, mask: 255.255.255.0, gw: 192.168.1.1
I (2033) wifi station: got ip:192.168.1.7
I (2033) wifi station: connected to ap SSID:CMCC-vVSQ password:vPp52YWM
```


## 注意事项

* 连接的WIFI热点，需要开启DHCP。且需要有2.4GHz的频段。

* 如无家庭或其他公共WIFI，可使用手机开启的热点。（注意：需要设置频段为2.4G，ESP32硬件不支持5G的WIFI频段）

* 此WIFI_station例程仅作了解即可，
