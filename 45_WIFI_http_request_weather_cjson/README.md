# 45_WIFI_http_request_weather_cjson

## 例程简介

此例程借助 `心知天气` 的HTTP-API，以`HTTP GET`的方式去获取天气（需申请个人账号，免费版每个账号可以请求 20次/分钟，可获得国内主要370个主要城市的今、明、后3日天气）。

账号密钥、API生成示例：[调试 心知天气api](https://blog.csdn.net/Mark_md/article/details/107696292?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163473939616780264031099%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163473939616780264031099&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29-1-107696292.pc_v2_rank_blog_default&utm_term=%E5%BF%83%E7%9F%A5%E5%A4%A9%E6%B0%94&spm=1018.2226.3001.4450)

示例接口：[获取合肥近5日天气(免费版只有3天)](http://api.seniverse.com/v3/weather/daily.json?key=S6eMmAGubL0Twlnxo&location=hefei&language=zh-Hans&unit=c&start=0&days=5)

官方在线文档：[心知天气 - 在线文档](https://seniverse.yuque.com/books/share/ded1e167-e35c-4669-8306-cf65c6e01dc0/uv4wnu)

JSON在线格式化工具：[JSON在线格式化](https://www.json.cn/#)

`menuconfig`中配置 连接的WIFI热点名称、WIFI密码。

ESP32上电运行后，会开启WIFI，设置为STA模式，用预设的WIFI热点名称、WIFI密码来连接热点。连接成功后，ESP32会通过log告知获得的IP、子网掩码、网关等信息。

连接到热点后，ESP32会建立Socket连接，通过80端口向`心知天气`发送请求天气信息的`HTTP GET`。接收到返回报文后，使用`cjson`进行格式解析，用log打印出未来天气情况。请求会以60分钟为周期刷新。

`cjson` 已被ESP-IDF集成在其中，可直接调用，目录位于`${IDF_PATH}\components\json\cJSON`。

ESP-IDF 使用开源 lwIP 轻量级 TCP/IP 协议栈。更多API介绍，详见乐鑫的在线文档：[ESP32 lwip](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/lwip.html)

心知天气的cJson解析已被封装到了 `http_get_weather.c.h` 中。


## 心知天气的HTTP-API返回的JSON示例
```json
{
    "results":[
        {
            "location":{
                "id":"WTEMH46Z5N09",
                "name":"合肥",
                "country":"CN",
                "path":"合肥,合肥,安徽,中国",
                "timezone":"Asia/Shanghai",
                "timezone_offset":"+08:00"
            },
            "daily":[
                {
                    "date":"2021-10-20",
                    "text_day":"中雨",
                    "code_day":"14",
                    "text_night":"小雨",
                    "code_night":"13",
                    "high":"13",
                    "low":"8",
                    "rainfall":"13.88",
                    "precip":"1.00",
                    "wind_direction":"东北",
                    "wind_direction_degree":"45",
                    "wind_speed":"23.4",
                    "wind_scale":"4",
                    "humidity":"90"
                },
                {
                    "date":"2021-10-21",
                    "text_day":"阴",
                    "code_day":"9",
                    "text_night":"阴",
                    "code_night":"9",
                    "high":"17",
                    "low":"8",
                    "rainfall":"0.00",
                    "precip":"0.00",
                    "wind_direction":"北",
                    "wind_direction_degree":"0",
                    "wind_speed":"8.4",
                    "wind_scale":"2",
                    "humidity":"89"
                },
                {
                    "date":"2021-10-22",
                    "text_day":"阴",
                    "code_day":"9",
                    "text_night":"阴",
                    "code_night":"9",
                    "high":"18",
                    "low":"9",
                    "rainfall":"0.00",
                    "precip":"0.00",
                    "wind_direction":"北",
                    "wind_direction_degree":"0",
                    "wind_speed":"3.0",
                    "wind_scale":"1",
                    "humidity":"84"
                }
            ],
            "last_update":"2021-10-20T08:00:00+08:00"
        }
    ]
}
```


## JSON简介

`JSON`(JavaScript Object Notation)是一种轻量级的数据交换格式。它采用完全独立于语言的文本格式来存储和表示数据，易于人阅读和编写，同时也易于机器解析和生成。

`JSON` 的语法基本上可以视为 `JavaScript` 语法的一个子集，包括以下内容：

- 数据使用`键值对`的方式表示，键、值之间使用 `:` 相隔，键值对间使用 `,` 分割。如：
```json
    "date":"2021-10-20",
    "text_day":"中雨",
```
- '{}'大括号用来保存对象。
```json
    "location":{
        "id":"WTEMH46Z5N09",
        "name":"合肥",
        "country":"CN",
        "path":"合肥,合肥,安徽,中国",
        "timezone":"Asia/Shanghai",
        "timezone_offset":"+08:00"
    }
```
- `[]`方括号保存数组，数组值间使用 `,` 分割。
```json
    "daily":[
        {
            "date":"2021-10-20",
            "text_day":"中雨",
            "code_day":"14",
            "text_night":"小雨",
            "code_night":"13",
            "high":"13",
            "low":"8",
            "rainfall":"13.88",
            "precip":"1.00",
            "wind_direction":"东北",
            "wind_direction_degree":"45",
            "wind_speed":"23.4",
            "wind_scale":"4",
            "humidity":"90"
        },
        {
            "date":"2021-10-21",
            "text_day":"阴",
            "code_day":"9",
            "text_night":"阴",
            "code_night":"9",
            "high":"17",
            "low":"8",
            "rainfall":"0.00",
            "precip":"0.00",
            "wind_direction":"北",
            "wind_direction_degree":"0",
            "wind_speed":"8.4",
            "wind_scale":"2",
            "humidity":"89"
        },
        {
            "date":"2021-10-22",
            "text_day":"阴",
            "code_day":"9",
            "text_night":"阴",
            "code_night":"9",
            "high":"18",
            "low":"9",
            "rainfall":"0.00",
            "precip":"0.00",
            "wind_direction":"北",
            "wind_direction_degree":"0",
            "wind_speed":"3.0",
            "wind_scale":"1",
            "humidity":"84"
        }
    ]
```


## cJSON简介

cJSON是使用C语言编写，用来创建、解析JSON文件的库。cJSON是一个轻巧、方便、单文件、简单的可以作为ANSI-C标准的JSON解析器。

常用解析api：
```c
// 解析JSON数据，如果格式正确，则返回一JSON对象指针，错误返回NULL。该函数会通过malloc()函数在内存中开辟一个空间，使用完成需要手动释放。
cJSON * cJSON_Parse(const char *value);

// 根据键名称“string”，获取对应的值。 不区分大小写。
cJSON * cJSON_GetObjectItem(const cJSON * const object, const char * const string);

// 获取数组长度（数组中的项目数）
int cJSON_GetArraySize(const cJSON *array);

// 获取数组成员（从数组“array”中检索项目编号“index”。 如果不成功，则返回 NULL。）
cJSON * cJSON_GetArrayItem(const cJSON *array, int index);

// 通过cJSON_Delete()，释放cJSON_Parse()分配出来的内存
void cJSON_Delete(cJSON *c);
```


## 运行现象

编译源码前，需要配置`menuconfig`中的 "WiFi SSID" 和 "WiFi Password"，与连接的WIFI热点的名称和密码一致。

如编译正常，运行后会输出如下内容：

```
I (0) cpu_start: App cpu up.
I (437) heap_init: Initializing. RAM available for dynamic allocation:
I (444) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (450) heap_init: At 3FFB7858 len 000287A8 (161 KiB): DRAM
I (457) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (463) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (469) heap_init: At 400955FC len 0000AA04 (42 KiB): IRAM
I (476) cpu_start: Pro cpu start user code
I (494) spi_flash: detected chip: generic
I (495) spi_flash: flash io: dio
I (495) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (504) gpio: GPIO[33]| InputEn: 1| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (554) wifi:wifi driver task: 3ffc0734, prio:23, stack:6656, core=0
I (554) system_api: Base MAC address is not set
I (554) system_api: read default base MAC address from EFUSE
I (574) wifi:wifi firmware version: bb6888c
I (574) wifi:wifi certification version: v7.0
I (574) wifi:config NVS flash: enabled
I (574) wifi:config nano formating: disabled
I (574) wifi:Init data frame dynamic rx buffer num: 32
I (584) wifi:Init management frame dynamic rx buffer num: 32
I (584) wifi:Init management short buffer num: 32
I (594) wifi:Init dynamic tx buffer num: 32
I (594) wifi:Init static rx buffer size: 1600
I (604) wifi:Init static rx buffer num: 10
I (604) wifi:Init dynamic rx buffer num: 32
I (604) wifi_init: rx ba win: 6
I (614) wifi_init: tcpip mbox: 32
I (614) wifi_init: udp mbox: 6
I (624) wifi_init: tcp mbox: 6
I (624) wifi_init: tcp tx win: 5744
I (624) wifi_init: tcp rx win: 5744
I (634) wifi_init: tcp mss: 1440
I (634) wifi_init: WiFi IRAM OP enabled
I (644) wifi_init: WiFi RX IRAM OP enabled
I (644) example_connect: Connecting to CMCC-vVSQ...
I (654) phy_init: phy_version 4660,0162888,Dec 23 2020
I (754) wifi:mode : sta (08:3a:f2:4a:3f:0c)
I (754) example_connect: Waiting for IP(s)
I (764) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,0>, prof:1
I (1534) wifi:state: init -> auth (b0)
I (1544) wifi:state: auth -> assoc (0)
I (1554) wifi:state: assoc -> run (10)
I (1564) wifi:connected with CMCC-vVSQ, aid = 4, channel 1, BW20, bssid = 2c:43:be:1c:fb:ff
I (1574) wifi:security: WPA2-PSK, phy: bgn, rssi: -67
I (1574) wifi:pm start, type: 1

I (1574) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (2544) esp_netif_handlers: example_connect: sta ip: 192.168.1.7, mask: 255.255.255.0, gw: 192.168.1.1
I (2544) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.1.7
I (3544) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:0a3a:f2ff:fe4a:3f0c, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (3544) example_connect: Connected to example_connect: sta
I (3554) example_connect: - IPv4 address: 192.168.1.7
I (3554) example_connect: - IPv6 address: fe80:0000:0000:0000:0a3a:f2ff:fe4a:3f0c, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (3584) example: DNS lookup succeeded. IP=116.62.81.138
I (3584) example: ... allocated socket
I (3664) example: ... connected
I (3664) example: ... socket send success:GET /v3/weather/daily.json?key=S6eMmAGubL0Twlnxo&location=hefei&language=zh-Hans&unit=c&start=0&days=5 HTTP/1.0
Host: api.seniverse.com
User-Agent: esp-idf/4.2-esp32


I (3674) example: ... set socket receiving timeout success
HTTP/1.1 200 OK
Date: Thu, 21 Oct 2021 13:59:16 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 981
Connection: close
X-Instance-Id: 15823285-716b-470e-9e82-f6f9d04e7729
X-RateLimit-Limit-minute: 20
X-RateLimit-Remaining-minute: 19
X-Powered-By: Express
ETag: W/"3d5-QRmZ9hb1WTPhG4sg1jYrQ8X6KFg"
X-Kong-Upstream-Latency: 2
X-Kong-Proxy-Latency: 26
Via: kong/0.14.1

{"results":[{"location":{"id":"WTEMH46Z5N09","name":"合肥","country":"CN","path":"合肥,合肥,安徽,中国","timezone":"Asia/Shanghai","timezone_offset":"+08:00"},"daily":[{"date":"2021-10-21","text_day":"雾
","code_day":"30","text_night":"多云","code_night":"4","high":"16","low":"8","rainfall":"0.00","precip":"0.00","wind_direction":"北","wind_direction_degree":"0","wind_speed":"23.4","wind_scale":"4","humidity":"90"},{"date":"2021-10-22","text_day":"阴","code_day":"9","text_night":"阴","code_night":"9","high":"18","low":"8","rainfall":"0.00","precip":"0.00","wind_direction":"北","wind_direction_degree":"0","wind_speed":"8.4","wind_scale":"2","humidity":"86"},{"date":"2021-10-23","text_day":"多云","code_day":"4","text_night":"多云","code_night":"4","high":"19","low":"9","rainfall":"0.00","precip":"0.00","wind_direction":"东","wind_direction_degree":"90","wind_speed":"3.0","wind_scale":"1","humidity":"72"}],"last_update":"2021-10-21T08:00:00+08:00"}]}I (3824) example: r= 0, rnum= 1379

cJSON_GetArraySize: size=1

 合肥,合肥,安徽,中国
未来3日天气:
2021-10-21      8 - 16 ℃        雾  多云
2021-10-22      8 - 18 ℃        阴  阴
2021-10-23      9 - 19 ℃        多云  多云
W (3793) example: [APP] Free memory: 231852 bytes
I (3844) example: ... done reading from socket. Last read return=0 errno=128.
I (3854) example: 3600... 
I (4854) example: 3599... 
I (5854) example: 3598... 
I (6854) example: 3597... 
I (7854) example: 3596... 
I (8854) example: 3595...
...
```


## 注意事项

* 使用cJson 解析/生成JSON字段后，务必释放内存。避免内存溢出。如程序运行时发现刚开始好用，但过一会儿WIFI一直断连，首先怀疑cJson的用法问题。已在Demo的任务中，持续性调试输出剩余内存容量，是为了及时发现内存溢出的现象。

* 心知天气需申请个人账号，免费版每个账号可以请求 20次/分钟，可获得国内主要370个主要城市的今、明、后3日天气。

* 鉴于接口不是特别稳定偶尔会抽风，而且是收费的，已在下个Demo中替换为了其他站点的免费api。

* 注意请求不能过于频繁，免费版每个账号仅可以请求 20次/分钟。

* app_main.c:185~197：本Demo已对ESP-IDF的 http_requset 中的 http read 部分，进行了改进升级。能够将以前的socket分段接收合成为一整段，并可统计一帧http报文总长度。

* ESP32连接的WIFI热点，需要有2.4GHz的频段。
