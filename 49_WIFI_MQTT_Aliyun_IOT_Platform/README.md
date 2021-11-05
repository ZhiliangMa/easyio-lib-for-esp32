# 49_WIFI_MQTT_Aliyun_IOT_Platform

## 例程简介

上两个Demo分别用到了cJson、MQTT、mbedtls，至此对ESP32常用的WIFI知识基本介绍过半，本节将以前内容综合运用。MQTT接入阿里云物联网平台，了解基础的物联网设备通信流程。

主要内容有：

1. 注册阿里云开发者账户，进入`物联网平台`管理界面，创建公共实例，创建产品、多个设备。完善产品的`物模型`并发布上线。

【本Demo用到的物模型属性有：当前温度(CurrentTemperature)、当前湿度(CurrentHumidity)、光照度(mlux)、当前电压(CurrentVoltage)、状态灯开关(StatusLightSwitch)。将以上全部添加】

2. 查阅阿里云物联网平台的[在线手册](https://help.aliyun.com/document_detail/73742.html?spm=a2c4g.11186623.6.614.c92e3d45d80aqG)，熟悉最基础的`一机一密`注册认证方式，熟悉用`设备证书(三元组)`生成`MQTT客户端配置`的方法。并使用 `MQTT.fx` 模拟客户端接入物联网平台，对接入过程做简单了解。

3. 使用 `MQTT.fx` 模拟客户端，订阅`属性上报`、`属性设置`这两个主题；云平台使用`在线调试`、`设备模拟器`与MQTT.fx通信，模拟数据上传和指令下发。

4. 关掉`MQTT.fx`客户端。将`ESP32开发板`作为实体设备节点，连接WIFI获取网络连接，使用MQTT连接阿里云物联网平台，订阅相关主题。

5. `ESP32开发板`上报温湿度、光照度、电池电压 等传感器信息，用户可在云平台的控制台，下发命令，控制设备LED的运行状态。

以上就是本节相关的内容，内容略多，过程较繁琐，但ESP32的代码量较精简。

得益于以前对设备驱动的良好封装，复用了以前的`SHT30温湿度传感器`、`ADC光照度`、`电池电压`、`LED`、`按键`的驱动，几行代码搞定传感器数据。

接着沿用上两节的`MQTT事件处理`框架，只需额外添加 `阿里云MQTT设备认证`、`对接阿里云物模型的JSON解析/上报`，即可实现完整的`物联网控制`流程。

而以上内容，也同样被封装为三个.c.h，他们为 `mbedtls_encrypt`、`mqtt_passwd_generate`、`aliyun_iot_event`，分别用于 `HMAC加密`、`阿里云MQTT登录方式及密码合成`、`阿里云物联网平台-物模型属性JSON上报、解析控制指令` 。


***

# 阿里云物联网平台的使用

以下过程我就直接放链接了，都是以前写的博文。阿里云物联网平台的操作方式这两年没怎么变，可放心食用。

进入[阿里云-物联网平台](https://iot.console.aliyun.com/lk/summary/new)，注册开发者账号。

[注册产品、设备，完善物模型，查看设备证书三元组](https://blog.csdn.net/Mark_md/article/details/108051182?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163577752316780269893156%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163577752316780269893156&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29_name-6-108051182.pc_v2_rank_blog_default&utm_term=%E9%98%BF%E9%87%8C%E4%BA%91&spm=1018.2226.3001.4450)。

【本Demo用到的物模型属性有：当前温度(CurrentTemperature)、当前湿度(CurrentHumidity)、光照度(mlux)、当前电压(CurrentVoltage)、状态灯开关(StatusLightSwitch)。将以上全部添加】

在线文档 [如何编辑物模型草稿、发布上线](https://help.aliyun.com/document_detail/88241.html)。

在线手册 [公共实例与企业实例的区别](https://help.aliyun.com/document_detail/147356.html)。

查看在线手册，熟悉 [`一机一密`注册认证方式](https://help.aliyun.com/document_detail/73742.html?spm=a2c4g.11186623.6.614.c92e3d45d80aqG)。

MQTT模拟设备 [MQTT.fx客户端MQTT接入阿里云物联网平台，登录、订阅、发布消息](https://blog.csdn.net/Mark_md/article/details/108316694?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163577752316780269893156%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163577752316780269893156&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29_name-4-108316694.pc_v2_rank_blog_default&utm_term=%E9%98%BF%E9%87%8C%E4%BA%91&spm=1018.2226.3001.4450)。

在线手册 [使用MQTT.fx接入物联网平台](https://help.aliyun.com/document_detail/140507.html)。

以上的调试的操作不是很规范，建议用`监控运维`下的`在线调试`和`设备模拟器`进行先行测试，然后再对上发的报文进行编辑。

在不知道上报消息格式的情况下，可以使用云平台的`设备模拟器`。获取到模拟的上报数据后，MQTT.fx仿照着格式进行上发。

熟悉Python的，也可试下[Python模拟智能开关设备MQTT接入阿里云物联网平台](https://blog.csdn.net/Mark_md/article/details/117446455?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163530037016780269892307%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163530037016780269892307&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29-1-117446455.pc_v2_rank_blog_default&utm_term=pycharm&spm=1018.2226.3001.4450)。


***

# 本节Demo代码的使用方法

了解上述操作后，对阿里云物联网平台的接入、管理有了初步认识。即可将`设备证书`三元组复制，粘贴到本Demo的`main.c`中的代码内。

并根据自己的设备证书，进行修改。如下：
```
/*
{
  "ProductKey": "a1WFMPFCsrp",
  "DeviceName": "dev_0002",
  "DeviceSecret": "605ed1663e9d945e014d462e8f201e98"
}
*/
// 设备证书（ProductKey、DeviceName和DeviceSecret），三元组
char productKey[20] = "a1WFMPFCsrp";
char deviceName[20] = "dev_0002";
char deviceSecret[40] = "605ed1663e9d945e014d462e8f201e98"; // 32字节长度
char regionId[20] = "cn-shanghai"; // 接入的服务器地址
```

程序会根据设备证书，自动生成 `阿里云MQTT登录方式`，读取板载SHT30温湿度传感器、光照传感器、电池电压进行上报，并对下发内容做解析，以控制LED运行状态。

接着打开menuconfig，配置`ESP32开发板`连接的WIFI热点名称和密码。

编译、下载、运行。

打开`阿里云物联网平台`的控制台，点击`设备`，查看`物模型数据`，打开`实时刷新`。查看上报的数据，改变`ESP32开发板`的环境温湿度、亮度，看数据是否正确。

或者查看Vscode终端的输出，看与控制台的数据是否一致。

也可以图表的方式查看曲线。

因开发板板载锂电，这里在烧录过程序后，完全可以脱机运行。测试过运行此Demo，2000mAH电池可维持工作将近两天。

打开`在线调试`，改变`状态灯开关`的状态，`调试` - `设置`。观察板载LED是否响应。

LED几乎是秒响应，控制台可对设备进行`时延测试`，网络条件好的情况下一般为几十ms，对控制消息的响应速度十分迅速。

短按 / 长按 `BOOT`按键，查看LED运行状态，并观察控制台的 `状态灯开关` 的值变化。

至此，一个完整的 连接`阿里云-物联网平台`，可上报`温湿度`、`光照度`、`电池电压`，并可对LED进行本地、远程双向控制的物联网设备，就完成啦。

借助`easyio`驱动库，实现上面完整的逻辑，`main.c`中的代码算上注释才300行，不可谓不简洁。驱动部分都是照搬以前的Demo任务，即拿即用。



***

# 阿里云下发消息的格式示例

```
TOPIC = "/sys/a1WFMPFCsrp/dev_0001/thing/service/property/set"
{"method":"thing.service.property.set","id":"1871805845","params":{"StatusLightSwitch":0},"version":"1.0.0"}
```

其中的物模型属性，可由服务器下发，也可由设备上发，如`状态灯开关`，可由服务器控制板载LED运行状态，也可由按键控制LED运行状态后再上发告知服务器。

图形化APP怎么弄。

***

# 设备向阿里云上报消息的格式示例

上报温湿度的模拟消息。
```json
# 原文
{"id":1635524280792,"params":{"CurrentTemperature":22.86907958984375,"CurrentHumidity":40},"version":"1.0","method":"thing.event.property.post"}

# 格式化后
{
	"id": 1635524280792,
	"params": {
		"CurrentTemperature": 22.86907958984375,
		"CurrentHumidity": 40
	},
	"version": "1.0",
	"method": "thing.event.property.post"
}
```

***

# 一些常见问题

```
- Q:设备如果订阅了`属性上报`，会在发布消息后马上接收到相同的消息，而这消息对此设备无用，可不可以不订阅`属性上报`主题？ 
- A:不可以，设备不订阅`属性上报`，会无法上报消息。
```

```
- Q:json合成，如何限制小数位数？ 
-A:cJson对这种需求真的没办法。
```

```
- Q:为什么我没有订阅过这个主题，但还是会收到这个主题的消息？如`"/sys/a1wFylTxYeD/co_0001/thing/event/property/post_reply"`

- A:大概率是之前使用了该设备的 `设备模拟器`，使用阿里云的设备模拟器后，会默认订阅该设备的所有`Topic`，此时再切换为实际的`ESP32开发板`去连接，会多出很多无用的报文。(暂未找到快捷退订所有Topic的方法，所以建议创建一个设备专门用来做测试)
```

```
- Q:有没有快捷退订所有主题的方法？
- A:暂未找到。目前只能一条一条退订。
```

```
- Q:观察`设备`-`物模型数据`，打开实时刷新，为什么感觉延迟很高，不像是实时的？是不是MQTT不适合实时控制？
- A:这是因为网页所致，测试延迟可用`设备`-`设备信息`-`实时延迟`去测试，一般在100ms左右。或者通过`在线调试`去下发控制指令，能感受到ESP32开发板的LED对指令的响应非常迅速。MQTT的底层通信链路为TCP，是实时通信协议。
```

```
- Q:我在产品物模型中，又增加了几个属性，并在ESP32的代码中额外增加了上报和解析。为什么改动后，刚开始程序可以正常运行，但过了一段时间会导致WIFI一直断连呢？
- A:刚开始可以运行，运行一段时间出错，首先怀疑内存溢出。溢出的原因很可能是自己没使用好cJson，解析/生成JSON字段后，务必释放内存。
本Demo代码中，在 `mqtt_app_start_task` 任务中，持续性调试输出剩余内存容量，是为了及时发现内存溢出的现象。

ESP_LOGW(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size()); // 调试输出空闲内存的空间，及时定位因内存溢出而导致的故障
```

***

【】阿里云物联网平台如何用控件去设置图形化应用

***

## 运行现象

```
I (0) cpu_start: App cpu up.
I (511) heap_init: Initializing. RAM available for dynamic allocation:
I (518) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (524) heap_init: At 3FFB7FE8 len 00028018 (160 KiB): DRAM
I (530) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (537) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (543) heap_init: At 40096D20 len 000092E0 (36 KiB): IRAM
I (549) cpu_start: Pro cpu start user code
I (568) spi_flash: detected chip: generic
I (568) spi_flash: flash io: dio
I (568) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (576) MQTT: [APP] Startup..
I (576) gpio: GPIO[33]| InputEn: 1| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (586) MQTT: [APP] Free memory: 275136 bytes
I (596) MQTT: [APP] IDF version: v4.2.2-dirty
I (636) wifi:wifi driver task: 3ffc0dd8, prio:23, stack:6656, core=0
I (636) system_api: Base MAC address is not set
I (636) system_api: read default base MAC address from EFUSE
I (646) wifi:wifi firmware version: bb6888c
I (646) wifi:wifi certification version: v7.0
I (646) wifi:config NVS flash: enabled
I (646) wifi:config nano formating: disabled
I (656) wifi:Init data frame dynamic rx buffer num: 32
I (656) wifi:Init management frame dynamic rx buffer num: 32
I (666) wifi:Init management short buffer num: 32
I (666) wifi:Init dynamic tx buffer num: 32
I (676) wifi:Init static rx buffer size: 1600
I (676) wifi:Init static rx buffer num: 10
I (676) wifi:Init dynamic rx buffer num: 32
I (686) wifi_init: rx ba win: 6
I (686) wifi_init: tcpip mbox: 32
I (696) wifi_init: udp mbox: 6
I (696) wifi_init: tcp mbox: 6
I (696) wifi_init: tcp tx win: 5744
I (706) wifi_init: tcp rx win: 5744
I (706) wifi_init: tcp mss: 1440
I (716) wifi_init: WiFi IRAM OP enabled
I (716) wifi_init: WiFi RX IRAM OP enabled
I (726) example_connect: Connecting to CMCC-vVSQ...
I (726) phy_init: phy_version 4660,0162888,Dec 23 2020
I (836) wifi:mode : sta (08:3a:f2:4a:3f:88)
I (836) example_connect: Waiting for IP(s)
I (846) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,0>, prof:1
I (1616) wifi:state: init -> auth (b0)
I (1636) wifi:state: auth -> assoc (0)
I (1636) wifi:state: assoc -> run (10)
I (1656) wifi:connected with CMCC-vVSQ, aid = 3, channel 1, BW20, bssid = 2c:43:be:1c:fb:ff
I (1656) wifi:security: WPA2-PSK, phy: bgn, rssi: -62
I (1656) wifi:pm start, type: 1

I (1666) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (2616) esp_netif_handlers: example_connect: sta ip: 192.168.1.8, mask: 255.255.255.0, gw: 192.168.1.1
I (2616) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.1.8
I (3616) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:0a3a:f2ff:fe4a:3f88, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (3616) example_connect: Connected to example_connect: sta
I (3626) example_connect: - IPv4 address: 192.168.1.8
I (3626) example_connect: - IPv6 address: fe80:0000:0000:0000:0a3a:f2ff:fe4a:3f88, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (3646) mbedtls: ------------------------------ HMAC 加密、Base64 编码 ---------------

I (3646) gpio: GPIO[0]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:3 
I (3646) mbedtls: plain: clientIddev_0002deviceNamedev_0002productKeya1WFMPFCsrp
I (3666) mbedtls: key: 605ed1663e9d945e014d462e8f201e98
I (3676) MQTT: ligehLux: 142mV  v_bat: 4.223700V

I (3676) mbedtls: HMAC-SHA1: 
I (3676) MQTT: sht30 init ok.

I (3686) mbedtls: c31286f05e5c9cbc5bb4b9d24d16e6b01ca38552

I (3676) gpio: GPIO[27]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:3 
I (3696) mbedtls: HMAC-HEX: c31286f05e5c9cbc5bb4b9d24d16e6b01ca38552

I (3696) MQTT: temperature = 24.09 ℃, humidity = 50.06 %

I (3706) mbedtls: base64: wxKG8F5cnLxbtLnSTRbmsByjhVI=
E (3706) gpio: gpio_install_isr_service(438): GPIO isr service already installed
W (3726) mqPwd: mqttClientId: dev_0002|securemode=3,signmethod=hmacsha1|

W (3736) mqPwd: mqttUsername: dev_0002&a1WFMPFCsrp

W (3746) mqPwd: mqttPassword: c31286f05e5c9cbc5bb4b9d24d16e6b01ca38552

W (3746) mqPwd: mqttPasswordBase64: wxKG8F5cnLxbtLnSTRbmsByjhVI=

W (3756) mqPwd: brokerUrl: a1WFMPFCsrp.iot-as-mqtt.cn-shanghai.aliyuncs.com

W (3766) mqPwd: topic_post: /sys/a1WFMPFCsrp/dev_0002/thing/event/property/post

W (3776) mqPwd: topic_set: /sys/a1WFMPFCsrp/dev_0002/thing/service/property/set

I (3786) MQTT: Other event id:7
I (3916) MQTT: MQTT_EVENT_CONNECTED
I (3916) MQTT: sent subscribe successful, "/sys/a1WFMPFCsrp/dev_0002/thing/event/property/post", msg_id=23675
I (3926) MQTT: sent subscribe successful, "/sys/a1WFMPFCsrp/dev_0002/thing/service/property/set", msg_id=41665
I (3946) MQTT: MQTT_EVENT_SUBSCRIBED, msg_id=23675
I (3976) MQTT: MQTT_EVENT_SUBSCRIBED, msg_id=41665
I (4716) MQTT: ligehLux: 142mV  v_bat: 4.223700V

I (4726) MQTT: temperature = 24.07 ℃, humidity = 50.10 %

I (5726) MQTT: temperature = 24.06 ℃, humidity = 50.11 %

I (5746) MQTT: ligehLux: 142mV  v_bat: 4.223700V

W (5786) MQTT: [APP] Free memory: 206368 bytes
data:
{"id":1481765933,"params":{"CurrentTemperature":24.057373046875,"CurrentHumidity":50,"mlux":142,"CurrentVoltage":4.2237000465393066,"StatusLightSwitch":1},"version":"1.0","method":"thing.event.property.post"}
I (5846) MQTT: MQTT_EVENT_DATA
I (5846) MQTT: TOPIC = "/sys/a1WFMPFCsrp/dev_0002/thing/event/property/post"

I (5846) MQTT: DATA = "{"id":1481765933,"params":{"CurrentTemperature":24.057373046875,"CurrentHumidity":50,"mlux":142,"CurrentVoltage":4.2237000465393066,"StatusLightSwitch":1},"version":"1.0","method":"thing.event.property.post"}", num = 208

W (5876) aliyun: 
StatusLightSwitch: 1

I (6726) MQTT: temperature = 24.09 ℃, humidity = 50.01 %

I (6776) MQTT: ligehLux: 142mV  v_bat: 4.223700V

I (7726) MQTT: temperature = 24.06 ℃, humidity = 50.05 %

W (7796) MQTT: [APP] Free memory: 205384 bytes
data:
{"id":1085377743,"params":{"CurrentTemperature":24.057373046875,"CurrentHumidity":50,"mlux":142,"CurrentVoltage":4.2237000465393066,"StatusLightSwitch":1},"version":"1.0","method":"thing.event.property.post"}
I (7806) MQTT: ligehLux: 142mV  v_bat: 4.223700V

I (7846) MQTT: MQTT_EVENT_DATA
I (7846) MQTT: TOPIC = "/sys/a1WFMPFCsrp/dev_0002/thing/event/property/post"

I (7846) MQTT: DATA = "{"id":1085377743,"params":{"CurrentTemperature":24.057373046875,"CurrentHumidity":50,"mlux":142,"CurrentVoltage":4.2237000465393066,"StatusLightSwitch":1},"version":"1.0","method":"thing.event.property.post"}", num = 208

W (7876) aliyun: 
StatusLightSwitch: 1
```

***

## 注意事项

* 设备可订阅的主题，可在`产品` - `Topic类列表`中查看。

* 只要用过`设备模拟器`，该设备就会自动订阅所有主题。（实际设备只订阅Demo合成出的两条主题即可。其他的是另外的服务信息，可以节省流量费用）

* 一旦使用了`设备模拟器`，阿里云就会默认订阅该设备的所有`Topic`，此时再切换为实际的`ESP32开发板`去连接，会多出很多无用的报文。(暂未找到快捷退订所有Topic的方法，所以建议创建一个设备专门用来做测试)

* 阿里云物联网平台并不免费，`公共实例`也同样计费。计费分为 `消息通信费` + `连接时长费` 两部分。详见在线文档[计费方式](https://help.aliyun.com/document_detail/73701.html)

* 光照度来源于板载的光敏传感器，其信号输出方式为`电压值`，需要对照数据手册查表，才能得到实际的`LUX`光照强度。这里为简化过程，直接将mV电压值上传，并不是实际的光强。

* 如需精准的光照度，建议在I2C总线上挂载额外的光强传感器。

* 阿里云物联网平台的`设备-物模型`，来源于`产品-功能定义`。即同一产品下的所有设备共用一个`物模型`模板。如需增删功能，请到`产品-功能定义`下`编辑草稿`。

* 使用cJson 解析/生成JSON字段后，务必释放内存。避免内存溢出。如程序运行时发现刚开始好用，但过一会儿WIFI一直断连，首先怀疑cJson的用法问题。

* 其他几家大厂的物联网平台，其连接方式类似。阿里云物联网平台的设备属性资源较为丰富，文档更为齐全，对新手较友好。平台差异化对比可见：[阿里云、华为云和腾讯云等多家物联网平台的异同](https://www.cnblogs.com/yefanqiu/p/14500269.html)

* 如想定制自己的物联网平台，可从一些开源的入手，如 [thingsboard](https://thingsboard.io/)，[thingsboard中文网](http://www.ithingsboard.com/)
