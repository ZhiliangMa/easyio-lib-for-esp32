# 42_WIFI_udp_client

## 例程简介

此例程照搬ESP-IDF的 protocols -> sockets -> udp_client。

修改了`Kconfig.projbuild`，使得可以在`menuconfig`配置连接的 UDP的IP地址、端口号。

`menuconfig`中可配置 连接的WIFI热点名称、WIFI密码、WIFI网络通道、连接STA的最大连接数。

ESP32上电运行后，会开启WIFI，设置为STA模式，用预设的WIFI热点名称、WIFI密码来连接热点。连接成功后，ESP32会通过log告知获得的IP、子网掩码、网关等信息。

连接到热点后，ESP32会创建socket连接，以UDP的通信方式去发送消息，并将收到的消息原路返回。

ESP-IDF 使用开源 lwIP 轻量级 TCP/IP 协议栈。更多API介绍，详见乐鑫的在线文档：[ESP32 lwip](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/lwip.html)


# 运行现象

编译源码前，需要配置`menuconfig`中的 "WiFi SSID" 和 "WiFi Password"，与连接的WIFI热点的名称和密码一致。

查看自己电脑的IP地址，（windows环境使用ipconfig命令），配置`menuconfig`的IPV4 Address与本机IP相同。

如编译正常，运行后会输出如下内容：

连接到WIFI热点：

```
I (0) cpu_start: App cpu up.
I (430) heap_init: Initializing. RAM available for dynamic allocation:
I (437) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (443) heap_init: At 3FFB7828 len 000287D8 (161 KiB): DRAM
I (449) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (456) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (462) heap_init: At 400955FC len 0000AA04 (42 KiB): IRAM
I (468) cpu_start: Pro cpu start user code
I (487) spi_flash: detected chip: generic
I (487) spi_flash: flash io: dio
I (487) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (496) gpio: GPIO[33]| InputEn: 1| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (556) wifi:wifi driver task: 3ffc07ec, prio:23, stack:6656, core=0
I (556) system_api: Base MAC address is not set
I (556) system_api: read default base MAC address from EFUSE
I (576) wifi:wifi firmware version: bb6888c
I (576) wifi:wifi certification version: v7.0
I (576) wifi:config NVS flash: enabled
I (576) wifi:config nano formating: disabled
I (586) wifi:Init data frame dynamic rx buffer num: 32
I (586) wifi:Init management frame dynamic rx buffer num: 32
I (596) wifi:Init management short buffer num: 32
I (596) wifi:Init dynamic tx buffer num: 32
I (606) wifi:Init static rx buffer size: 1600
I (606) wifi:Init static rx buffer num: 10
I (606) wifi:Init dynamic rx buffer num: 32
I (616) wifi_init: rx ba win: 6
I (616) wifi_init: tcpip mbox: 32
I (626) wifi_init: udp mbox: 6
I (626) wifi_init: tcp mbox: 6
I (626) wifi_init: tcp tx win: 5744
I (636) wifi_init: tcp rx win: 5744
I (636) wifi_init: tcp mss: 1440
I (646) wifi_init: WiFi IRAM OP enabled
I (646) wifi_init: WiFi RX IRAM OP enabled
I (656) example_connect: Connecting to CMCC-vVSQ...
I (656) phy_init: phy_version 4660,0162888,Dec 23 2020
I (766) wifi:mode : sta (c4:dd:57:b8:fa:14)
I (766) example_connect: Waiting for IP(s)
I (776) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,0>, prof:1
I (3006) wifi:state: init -> auth (b0)
I (3016) wifi:state: auth -> assoc (0)
I (3026) wifi:state: assoc -> run (10)
I (3046) wifi:connected with CMCC-vVSQ, aid = 4, channel 1, BW20, bssid = 2c:43:be:1c:fb:ff
I (3046) wifi:security: WPA2-PSK, phy: bgn, rssi: -67
I (3046) wifi:pm start, type: 1

I (3136) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (4536) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:c6dd:57ff:feb8:fa14, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (4536) esp_netif_handlers: example_connect: sta ip: 192.168.1.7, mask: 255.255.255.0, gw: 192.168.1.1
I (4546) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.1.7
I (4556) example_connect: Connected to example_connect: sta
I (4566) example_connect: - IPv4 address: 192.168.1.7
I (4566) example_connect: - IPv6 address: fe80:0000:0000:0000:c6dd:57ff:feb8:fa14, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (4586) example: Socket created, sending to 192.168.1.6:3333
I (4586) example: Message sent
```

使用NetAssist.exe开启UDP，并发送消息：

```
I (35708) example: Socket accepted ip address: 192.168.1.6
I (38468) example: Received 5 bytes: hello
W (41638) example: Connection closed
I (41648) example: Socket listening
```


## 注意事项

* ESP-IDF提供了一种连接WIFI的简单方法，为`example_connect()`。example_connect() 会不断按照用户配置去连接WIFI并重试，直到连接上热点。

* example_connect() 会不断的连接WIFI并重试，直到连接上热点。

* `example_connect()` 函数的功能为按照menuconfig的配置项，去配置 Wi-Fi 或以太网的连接。阅读 examples/protocols/README.md 中的“建立 Wi-Fi 或以太网连接”部分，可以获取有关此功能的更多信息。

* `example_connect()` 函数位于 `/examples/common_components/protocol_examples_common/connect.c` 文件内。

* `protocol_examples_common.h` 和 `addr_from_stdin.h` 文件位于SDK目录下，`\examples\common_components\protocol_examples_common\include`的位置。

* 必须在 `CMakeLists.txt` 中添加这句 `set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)`。否则即使可以使用`example_connect()`，也会因为`menuconfig`中没有`EXAMPLE_WIFI_SSID`、`CONFIG_EXAMPLE_WIFI_PASSWORD`等WIFI热点信息，而导致运行错误。

* 必须修改`Kconfig.projbuild`文件，以便可以用`menuconfig`配置`IPV4 Address`、`Port`。

* ESP32连接的WIFI热点，需要有2.4GHz的频段。
