# 44_WIFI_http_request

## 例程简介

此例程照搬ESP-IDF的 protocols -> http_request。

`menuconfig`中可配置 连接的WIFI热点名称、WIFI密码。

可通过修改`app_main.c`中的宏定义`WEB_SERVER`，来修改HTTP访问的网页地址。

ESP32上电运行后，会开启WIFI，设置为STA模式，用预设的WIFI热点名称、WIFI密码来连接热点。连接成功后，ESP32会通过log告知获得的IP、子网掩码、网关等信息。

连接到热点后，ESP32会建立Socket连接，通过80端口发送`HTTP GET`请求，并接收网页内容。成功后，会以10s为周期不断刷新。

详细介绍可查阅 ESP-IDF目录下的的 protocols -> http_request 例程。

ESP-IDF 使用开源 lwIP 轻量级 TCP/IP 协议栈。更多API介绍，详见乐鑫的在线文档：[ESP32 lwip](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/lwip.html)


# 运行现象

编译源码前，需要配置`menuconfig`中的 "WiFi SSID" 和 "WiFi Password"，与连接的WIFI热点的名称和密码一致。

如编译正常，运行后会输出如下内容：

```
I (0) cpu_start: App cpu up.
I (434) heap_init: Initializing. RAM available for dynamic allocation:
I (441) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (447) heap_init: At 3FFB7840 len 000287C0 (161 KiB): DRAM
I (453) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (459) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (466) heap_init: At 400955FC len 0000AA04 (42 KiB): IRAM
I (472) cpu_start: Pro cpu start user code
I (490) spi_flash: detected chip: generic
I (491) spi_flash: flash io: dio
I (491) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (500) gpio: GPIO[33]| InputEn: 1| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (550) wifi:wifi driver task: 3ffc071c, prio:23, stack:6656, core=0
I (550) system_api: Base MAC address is not set
I (550) system_api: read default base MAC address from EFUSE
I (570) wifi:wifi firmware version: bb6888c
I (570) wifi:wifi certification version: v7.0
I (570) wifi:config NVS flash: enabled
I (570) wifi:config nano formating: disabled
I (570) wifi:Init data frame dynamic rx buffer num: 32
I (580) wifi:Init management frame dynamic rx buffer num: 32
I (580) wifi:Init management short buffer num: 32
I (590) wifi:Init dynamic tx buffer num: 32
I (590) wifi:Init static rx buffer size: 1600
I (600) wifi:Init static rx buffer num: 10
I (600) wifi:Init dynamic rx buffer num: 32
I (600) wifi_init: rx ba win: 6
I (610) wifi_init: tcpip mbox: 32
I (610) wifi_init: udp mbox: 6
I (620) wifi_init: tcp mbox: 6
I (620) wifi_init: tcp tx win: 5744
I (620) wifi_init: tcp rx win: 5744
I (630) wifi_init: tcp mss: 1440
I (630) wifi_init: WiFi IRAM OP enabled
I (640) wifi_init: WiFi RX IRAM OP enabled
I (640) example_connect: Connecting to CMCC-vVSQ...
I (650) phy_init: phy_version 4660,0162888,Dec 23 2020
I (750) wifi:mode : sta (08:3a:f2:4a:3f:0c)
I (750) example_connect: Waiting for IP(s)
I (760) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,0>, prof:1
I (2790) wifi:state: init -> auth (b0)
I (2800) wifi:state: auth -> assoc (0)
I (2810) wifi:state: assoc -> run (10)
I (2830) wifi:connected with CMCC-vVSQ, aid = 4, channel 1, BW20, bssid = 2c:43:be:1c:fb:ff
I (2830) wifi:security: WPA2-PSK, phy: bgn, rssi: -64
I (2830) wifi:pm start, type: 1

I (2840) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (4040) esp_netif_handlers: example_connect: sta ip: 192.168.1.7, mask: 255.255.255.0, gw: 192.168.1.1
I (4040) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.1.7
I (4540) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:0a3a:f2ff:fe4a:3f0c, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (4540) example_connect: Connected to example_connect: sta
I (4550) example_connect: - IPv4 address: 192.168.1.7
I (4550) example_connect: - IPv6 address: fe80:0000:0000:0000:0a3a:f2ff:fe4a:3f0c, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (4580) example: DNS lookup succeeded. IP=93.184.216.34
I (4580) example: ... allocated socket
I (4690) example: ... connected
I (4690) example: ... socket send success
I (4690) example: ... set socket receiving timeout success
HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Content-Length: 1256
Cache-Control: max-age=604800
Date: Wed, 20 Oct 2021 04:20:03 GMT
ETag: "3147526947+ident"
Expires: Wed, 27 Oct 2021 04:20:03 GMT
Last-Modified: Thu, 17 Oct 2019 07:18:26 GMT
Server: ECS (sab/572B)
Vary: Accept-Encoding
Age: 164567
Connection: close

<!doctype html>
<html>
<head>
    <title>Example Domain</title>

    <meta charset="utf-8" />
    <meta http-equiv="Content-type" content="text/html; charset=utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <style type="text/css">
    body {
        background-color: #f0f0f2;
        margin: 0;
        padding: 0;
        font-family: -apple-system, system-ui, BlinkMacSystemFont, "Segoe UI", "Open Sans", "Helvetica Neue", Helvetica, Arial, sans-serif;

    }
    div {
        width: 600px;
        margin: 5em auto;
        padding: 2em;
        background-color: #fdfdff;
        border-radius: 0.5em;
        box-shadow: 2px 3px 7px 2px rgba(0,0,0,0.02);
    }
    a:link, a:visited {
        color: #38488f;
        text-decoration: none;
    }
    @media (max-width: 700px) {
        div {
            margin: 0 auto;
            width: auto;
        }
    }
    </style>
</head>

<body>
<div>
    <h1>Example Domain</h1>
    <p>This domain is for use in illustrative examples in documents. You may use this
    domain in literature without prior coordination or asking for permission.</p>
    <p><a href="https://www.iana.org/domains/example">More information...</a></p>
</div>
</body>
</html>
I (4840) example: ... done reading from socket. Last read return=0 errno=128.
I (4850) example: 10... 
I (5850) example: 9... 
I (6850) example: 8... 
I (7850) example: 7...
...
```


## 注意事项

* ESP-IDF提供了一种连接WIFI的简单方法，为`example_connect()`。example_connect() 会不断按照用户配置去连接WIFI并重试，直到连接上热点。

* `protocol_examples_common.h` 和 `addr_from_stdin.h` 文件位于SDK目录下，`\examples\common_components\protocol_examples_common\include`的位置。

* `example_connect()` 函数的功能为按照menuconfig的配置项，去配置 Wi-Fi 或以太网的连接。阅读 examples/protocols/README.md 中的“建立 Wi-Fi 或以太网连接”部分，可以获取有关此功能的更多信息。

* `example_connect()` 函数位于 `/examples/common_components/protocol_examples_common/connect.c` 文件内。

* 必须在 `CMakeLists.txt` 中添加这句 `set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)`。否则即使可以使用`example_connect()`，也会因为`menuconfig`中没有`EXAMPLE_WIFI_SSID`、`CONFIG_EXAMPLE_WIFI_PASSWORD`等WIFI热点信息，而导致运行错误。

* ESP32连接的WIFI热点，需要有2.4GHz的频段。
