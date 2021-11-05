# 46_WIFI_http_request_weather_cjson_free_15days

## 例程简介

上一个Demo使用 `心知天气` 的HTTP-API去获取天气，不过鉴于接口不是非常稳定偶尔会抽风，且免费版的可获取信息较少，在这个Demo中替换为了其他站点的免费api。

sojson网的天气api可获得近15日的天气预报，信息量丰富，且不用注册个人账号，即来即用。重点在于免费，使用过程中需要注意每日请求量不要超过2000次，不然有可能封IP。

免费的天气api：[sojson网-免费天气api](https://www.sojson.com/blog/305.html)

示例接口：[获取合肥近15日天气](http://t.weather.itboy.net/api/weather/city/101220101)

国内城市ID列表：[city_code.json](https://github.com/baichengzhou/weather.api/blob/master/src/main/resources/citycode-2019-08-23.json)

一些其他的实用API:[Github public-apis](https://github.com/public-apis/public-apis)

JSON在线格式化工具：[JSON在线格式化](https://www.json.cn/#)

`menuconfig`中配置 连接的WIFI热点名称、WIFI密码。

ESP32上电运行后，会开启WIFI，设置为STA模式，用预设的WIFI热点名称、WIFI密码来连接热点。连接成功后，ESP32会通过log告知获得的IP、子网掩码、网关等信息。

连接到热点后，ESP32会建立Socket连接，通过80端口向`sojson网`发送请求天气信息的`HTTP GET`。接收到返回报文后，使用`cjson`进行格式解析，用log打印出未来天气情况。请求会以60分钟为周期刷新。

`cjson` 已被ESP-IDF集成在其中，可直接调用，目录位于`${IDF_PATH}\components\json\cJSON`。

ESP-IDF 使用开源 lwIP 轻量级 TCP/IP 协议栈。更多API介绍，详见乐鑫的在线文档：[ESP32 lwip](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/lwip.html)

sojson天气的cJson解析已被封装到了 `http_get_weather.c.h` 中。


## sojson网 的HTTP-API返回的JSON示例（15日天气）

因信息较为丰富，在接收时ESP32需要多申请一些内存。

```json
{
    "message":"success感谢又拍云(upyun.com)提供CDN赞助",
    "status":200,
    "date":"20211021",
    "time":"2021-10-21 14:30:00",
    "cityInfo":{
        "city":"合肥市",
        "citykey":"101220101",
        "parent":"安徽",
        "updateTime":"13:16"
    },
    "data":{
        "shidu":"62%",
        "pm25":36,
        "pm10":68,
        "quality":"良",
        "wendu":"14",
        "ganmao":"极少数敏感人群应减少户外活动",
        "forecast":[
            {
                "date":"21",
                "high":"高温 15℃",
                "low":"低温 8℃",
                "ymd":"2021-10-21",
                "week":"星期四",
                "sunrise":"06:19",
                "sunset":"17:33",
                "aqi":48,
                "fx":"西北风",
                "fl":"2级",
                "type":"多云",
                "notice":"阴晴之间，谨防紫外线侵扰"
            },
            {
                "date":"22",
                "high":"高温 18℃",
                "low":"低温 8℃",
                "ymd":"2021-10-22",
                "week":"星期五",
                "sunrise":"06:20",
                "sunset":"17:32",
                "aqi":38,
                "fx":"北风",
                "fl":"2级",
                "type":"阴",
                "notice":"不要被阴云遮挡住好心情"
            },
            {
                "date":"23",
                "high":"高温 18℃",
                "low":"低温 8℃",
                "ymd":"2021-10-23",
                "week":"星期六",
                "sunrise":"06:20",
                "sunset":"17:31",
                "aqi":28,
                "fx":"东风",
                "fl":"2级",
                "type":"多云",
                "notice":"阴晴之间，谨防紫外线侵扰"
            },
            {
                "date":"24",
                "high":"高温 19℃",
                "low":"低温 8℃",
                "ymd":"2021-10-24",
                "week":"星期日",
                "sunrise":"06:21",
                "sunset":"17:30",
                "aqi":30,
                "fx":"东风",
                "fl":"2级",
                "type":"阴",
                "notice":"不要被阴云遮挡住好心情"
            },
            {
                "date":"25",
                "high":"高温 19℃",
                "low":"低温 9℃",
                "ymd":"2021-10-25",
                "week":"星期一",
                "sunrise":"06:22",
                "sunset":"17:28",
                "aqi":31,
                "fx":"东风",
                "fl":"2级",
                "type":"多云",
                "notice":"阴晴之间，谨防紫外线侵扰"
            },
            {
                "date":"26",
                "high":"高温 20℃",
                "low":"低温 11℃",
                "ymd":"2021-10-26",
                "week":"星期二",
                "sunrise":"06:23",
                "sunset":"17:27",
                "aqi":32,
                "fx":"东风",
                "fl":"2级",
                "type":"晴",
                "notice":"愿你拥有比阳光明媚的心情"
            },
            {
                "date":"27",
                "high":"高温 19℃",
                "low":"低温 13℃",
                "ymd":"2021-10-27",
                "week":"星期三",
                "sunrise":"06:23",
                "sunset":"17:27",
                "aqi":31,
                "fx":"东风",
                "fl":"2级",
                "type":"多云",
                "notice":"阴晴之间，谨防紫外线侵扰"
            },
            {
                "date":"28",
                "high":"高温 19℃",
                "low":"低温 9℃",
                "ymd":"2021-10-28",
                "week":"星期四",
                "sunrise":"06:24",
                "sunset":"17:26",
                "aqi":30,
                "fx":"东北风",
                "fl":"2级",
                "type":"晴",
                "notice":"愿你拥有比阳光明媚的心情"
            },
            {
                "date":"29",
                "high":"高温 17℃",
                "low":"低温 9℃",
                "ymd":"2021-10-29",
                "week":"星期五",
                "sunrise":"06:25",
                "sunset":"17:25",
                "aqi":32,
                "fx":"东风",
                "fl":"2级",
                "type":"晴",
                "notice":"愿你拥有比阳光明媚的心情"
            },
            {
                "date":"30",
                "high":"高温 17℃",
                "low":"低温 10℃",
                "ymd":"2021-10-30",
                "week":"星期六",
                "sunrise":"06:26",
                "sunset":"17:24",
                "aqi":35,
                "fx":"东北风",
                "fl":"2级",
                "type":"多云",
                "notice":"阴晴之间，谨防紫外线侵扰"
            },
            {
                "date":"31",
                "high":"高温 19℃",
                "low":"低温 12℃",
                "ymd":"2021-10-31",
                "week":"星期日",
                "sunrise":"06:27",
                "sunset":"17:23",
                "aqi":34,
                "fx":"东北风",
                "fl":"2级",
                "type":"多云",
                "notice":"阴晴之间，谨防紫外线侵扰"
            },
            {
                "date":"01",
                "high":"高温 20℃",
                "low":"低温 14℃",
                "ymd":"2021-11-01",
                "week":"星期一",
                "sunrise":"06:27",
                "sunset":"17:22",
                "aqi":28,
                "fx":"东北风",
                "fl":"2级",
                "type":"小雨",
                "notice":"雨虽小，注意保暖别感冒"
            },
            {
                "date":"02",
                "high":"高温 20℃",
                "low":"低温 13℃",
                "ymd":"2021-11-02",
                "week":"星期二",
                "sunrise":"06:28",
                "sunset":"17:21",
                "aqi":35,
                "fx":"北风",
                "fl":"2级",
                "type":"多云",
                "notice":"阴晴之间，谨防紫外线侵扰"
            },
            {
                "date":"03",
                "high":"高温 20℃",
                "low":"低温 14℃",
                "ymd":"2021-11-03",
                "week":"星期三",
                "sunrise":"06:29",
                "sunset":"17:20",
                "aqi":41,
                "fx":"东风",
                "fl":"1级",
                "type":"多云",
                "notice":"阴晴之间，谨防紫外线侵扰"
            },
            {
                "date":"04",
                "high":"高温 20℃",
                "low":"低温 13℃",
                "ymd":"2021-11-04",
                "week":"星期四",
                "sunrise":"06:30",
                "sunset":"17:19",
                "aqi":29,
                "fx":"北风",
                "fl":"1级",
                "type":"阴",
                "notice":"不要被阴云遮挡住好心情"
            }
        ],
        "yesterday":{
            "date":"20",
            "high":"高温 11℃",
            "low":"低温 8℃",
            "ymd":"2021-10-20",
            "week":"星期三",
            "sunrise":"06:18",
            "sunset":"17:34",
            "aqi":26,
            "fx":"东北风",
            "fl":"3级",
            "type":"中雨",
            "notice":"记得随身携带雨伞哦"
        }
    }
}
```


## JSON简介

`JSON`(JavaScript Object Notation)是一种轻量级的数据交换格式。它采用完全独立于语言的文本格式来存储和表示数据，易于人阅读和编写，同时也易于机器解析和生成。

`JSON` 的语法基本上可以视为 `JavaScript` 语法的一个子集，包括以下内容：

- 数据使用`键值对`的方式表示，键值之间使用 `:` 相隔，键值对间使用 `,` 分割。如：
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
I (438) heap_init: Initializing. RAM available for dynamic allocation:
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
I (1574) wifi:connected with CMCC-vVSQ, aid = 4, channel 1, BW20, bssid = 2c:43:be:1c:fb:ff
I (1574) wifi:security: WPA2-PSK, phy: bgn, rssi: -61
I (1574) wifi:pm start, type: 1

I (1664) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (3044) esp_netif_handlers: example_connect: sta ip: 192.168.1.7, mask: 255.255.255.0, gw: 192.168.1.1
I (3044) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.1.7
I (3544) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:0a3a:f2ff:fe4a:3f0c, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (3544) example_connect: Connected to example_connect: sta
I (3554) example_connect: - IPv4 address: 192.168.1.7
I (3554) example_connect: - IPv6 address: fe80:0000:0000:0000:0a3a:f2ff:fe4a:3f0c, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (3584) example: DNS lookup succeeded. IP=183.240.60.178
I (3584) example: ... allocated socket
I (3614) example: ... connected
I (3624) example: ... socket send success:GET /api/weather/city/101220101 HTTP/1.0
Host: t.weather.itboy.net
User-Agent: esp-idf/4.2-esp32


I (3624) example: ... set socket receiving timeout success
HTTP/1.1 200 OK
Server: marco/2.14
Date: Thu, 21 Oct 2021 13:12:11 GMT
Content-Type: application/json;charset=UTF-8
Connection: close
Vary: Accept-Encoding
X-Source: C/200
X-Real-Ip: 1.12.243.191
Accept-Ranges: bytes
Expires: Thu, 21 Oct 2021 13:20:00 GMT
Cache-Control: max-age=3000
Age: 2531
X-Request-Id: 1411b84374829f247b7d6d42122e723d
Via: S.mix-js-czx2-047, T.47.H, V.mix-js-czx2-046, T.100.M, M.cmn-gd-fuo-107

{"message":"success感谢又拍云(upyun.com)提供CDN赞助","status":200,"date":"20211021","time":"2021-10-21 20:30:00","cityInfo":{"city":"合肥市","citykey":"101220101","parent":"安徽","updateTime":"18:00"},"data":{"shidu":"74%","pm25":27.0,"pm10":61.0,"quality":"良","wendu":"13","ganmao":"极少数敏感人群应减少户外活动","forecast":[{"date":"21","high":"高温 15℃","low":"低温 8℃","ymd":"2021-10-21","week":"星 
期四","sunrise":"06:19","sunset":"17:33","aqi":44,"fx":"西北风","fl":"2级","type":"阴","notice":"不要被阴云遮挡住好心情"},{"date":"22","high":"高温 18℃","low":"低温 7℃","ymd":"2021-10-22","week":"星期五
","sunrise":"06:20","sunset":"17:32","aqi":37,"fx":"北风","fl":"2级","type":"阴","notice":"不要被阴云遮挡住好心情"},{"date":"23","high":"高温 18℃","low":"低温 8℃","ymd":"2021-10-23","week":"星期六","sunrise":"06:20","sunset":"17:31","aqi":31,"fx":"东风","fl":"2级","type":"多云","notice":"阴晴之间，谨防紫外线侵扰"},{"date":"24","high":"高温 19℃","low":"低温 8℃","ymd":"2021-10-24","week":"星期日","sunrise":"06:21","sunset":"17:30","aqi":34,"fx":"东风","fl":"2级","type":"阴","notice":"不要被阴云遮挡住好心情"},{"date":"25","high":"高温 19℃","low":"低温 9℃","ymd":"2021-10-25","week":"星期一","sunrise":"06:22","sunset":"17:28","aqi":33,"fx":"东风","fl":"2级","type":"多云","notice":"阴晴之间，谨防紫外线侵扰"},{"date":"26","high":"高温 20℃","low":"低温 11℃","ymd":"2021-10-26","week":"星期二","sunrise":"06:23","sunset":"17:27","aqi":33,"fx":"东风","fl":"2级","type":"晴","notice":"愿你拥有比阳光明媚的心情"},{"date":"27","high":"高温 19℃","low":"低温 13℃","ymd":"2021-10-27","week":"星期三","sunrise":"06:23","sunset":"17:27","aqi":33,"fx":"东风","fl":"2级","type":"多云","notice":"阴晴之间，谨防紫外线侵扰"},{"date":"28","high":"高温 19℃","low":"低温 9℃","ymd":"2021-10-28","week":"星期四","sunrise":"06:24","sunset":"17:26","aqi":31,"fx":"东北风","fl":"2级","type":"晴","notice":"愿你拥有比阳光明媚的心情"},{"date":"29","high":"高温 17℃","low":"低温 9℃","ymd":"2021-10-29","week":"星期五","sunrise":"06:25","sunset":"17:25","aqi":33,"fx":"东北风","fl":"2级","type":"晴","notice":"愿你拥有比阳光明媚的心情"},{"date":"30","high":"高温 17℃","low":"低温 10℃","ymd":"2021-10-30","week":"星期六","sunrise":"06:26","sunset":"17:24","aqi":31,"fx":"东风","fl":"2级","type":"多云","notice":"阴晴之间，谨防紫外线侵扰"},{"date":"31","high":"高温 19℃","low":"低温 12℃","ymd":"2021-10-31","week":"星期日","sunrise":"06:27","sunset":"17:23","aqi":32,"fx":"东风","fl":"2级","type":"阴","notice":"不要被阴云遮挡住好心情"},{"date":"01","high":"高温 20℃","low":"低温 14℃","ymd":"2021-11-01","week":"星期一","sunrise":"06:27","sunset":"17:22","aqi":33,"fx":"东北风","fl":"3级","type":"阴","notice":"不要被阴云遮挡住好心情"},{"date":"02","high":"高温 20℃","low":"低温 13℃","ymd":"2021-11-02","week":"星期二","sunrise":"06:28","sunset":"17:21","aqi":34,"fx":"北风","fl":"2级","type":"阴","notice":"不要被阴云遮挡住好心情"},{"date":"03","high":"高温 20℃","low":"低温 14℃","ymd":"2021-11-03","week":"星期三","sunrise":"06:29","sunset":"17:20","aqi":38,"fx":"西北风","fl":"1级","type":"晴","notice":"愿你拥有比阳光明媚的心情"},{"date":"04","high":"高温 20℃","low":"低温 13℃","ymd":"2021-11-04","week":"星期四","sunrise":"06:30","sunset":"17:19","aqi":29,"fx":"东南风","fl":"2级","type":"阴","notice":"不要被阴云遮挡住好心情"}],"yesterday":{"date":"20","high":"高温 11℃","low":"低温 8℃","ymd":"2021-10-20","week":"星期三","sunrise":"06:18","sunset":"17:34","aqi":26,"fx":"东北风","fl":"3级","type":"中雨","notice":"记得随身携带雨伞哦"}}}I (4194) example: r= 0, rnum= 4418

20211021 - 安徽 - 合肥市
当前: 13 ℃ - 湿度: 74% - PM2.5: 27 - AQI: 良
未来15日天气:
2021-10-21 - 星期四 - 阴 - 低温 8℃ - 高温 15℃ - 西北风 - 2级 - AQI: 44
2021-10-22 - 星期五 - 阴 - 低温 7℃ - 高温 18℃ - 北风 - 2级 - AQI: 37
2021-10-23 - 星期六 - 多云 - 低温 8℃ - 高温 18℃ - 东风 - 2级 - AQI: 31
2021-10-24 - 星期日 - 阴 - 低温 8℃ - 高温 19℃ - 东风 - 2级 - AQI: 34
2021-10-25 - 星期一 - 多云 - 低温 9℃ - 高温 19℃ - 东风 - 2级 - AQI: 33
2021-10-26 - 星期二 - 晴 - 低温 11℃ - 高温 20℃ - 东风 - 2级 - AQI: 33
2021-10-27 - 星期三 - 多云 - 低温 13℃ - 高温 19℃ - 东风 - 2级 - AQI: 33
2021-10-28 - 星期四 - 晴 - 低温 9℃ - 高温 19℃ - 东北风 - 2级 - AQI: 31
2021-10-29 - 星期五 - 晴 - 低温 9℃ - 高温 17℃ - 东北风 - 2级 - AQI: 33
2021-10-30 - 星期六 - 多云 - 低温 10℃ - 高温 17℃ - 东风 - 2级 - AQI: 31
2021-10-31 - 星期日 - 阴 - 低温 12℃ - 高温 19℃ - 东风 - 2级 - AQI: 32
2021-11-01 - 星期一 - 阴 - 低温 14℃ - 高温 20℃ - 东北风 - 3级 - AQI: 33
2021-11-02 - 星期二 - 阴 - 低温 13℃ - 高温 20℃ - 北风 - 2级 - AQI: 34
2021-11-03 - 星期三 - 晴 - 低温 14℃ - 高温 20℃ - 西北风 - 1级 - AQI: 38
2021-11-04 - 星期四 - 阴 - 低温 13℃ - 高温 20℃ - 东南风 - 2级 - AQI: 29
W (32663) example: [APP] Free memory: 227776 bytes
I (4324) example: ... done reading from socket. Last read return=0 errno=128.
I (4334) example: 3600... 
I (5334) example: 3599... 
I (6334) example: 3598... 
I (7334) example: 3597... 
I (8334) example: 3596... 
I (9334) example: 3595...
...
```


## 注意事项

* 使用cJson 解析/生成JSON字段后，务必释放内存。避免内存溢出。如程序运行时发现刚开始好用，但过一会儿WIFI一直断连，首先怀疑cJson的用法问题。已在Demo的任务中，持续性调试输出剩余内存容量，是为了及时发现内存溢出的现象。

* sojson网不需要注册账号，仅注意请求不能过于频繁。

* sojson网返回的天气报文中，`data -> pm25` 和 `data -> forecast[i] -> aqi` 需要被解析为 `valueint`，如果解析为 valuestring，会在运行后报错重启。

* app_main.c:220~232：本Demo已对ESP-IDF的 http_requset 中的 http read 部分，进行了改进升级。能够将以前的socket分段接收合成为一整段，并可统计一帧http报文总长度。

* ESP32连接的WIFI热点，需要有2.4GHz的频段。
