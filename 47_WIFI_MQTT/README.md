# 47_WIFI_MQTT

## 例程简介

MQTT是一种基于 `发布/订阅` 模式的轻量级通信协议。MQTT专门针对 `物联网设备` 开发，是一种 `低开销、低带宽占用的即时通讯协议`。该协议构建于 TCP/IP 上，旨在为低带宽和不稳定网络环境中的物联网设备，提供可靠的网络服务。它的设计思想是简单、开放、规范，易于实现，这些特点使其非常适合 机器间通信（M2M）、物联网（IoT）等场景。因其协议简单、数据流量开销低、时延低、对网络条件的容忍度高等特点，特别适合于硬件受限的嵌入式设备。MQTT 最大优点在于，可以以极少的代码和有限的网络带宽，为远程设备连接提供实时可靠的消息服务。

本Demo由乐鑫官方MQTT-Demo深度改进，将`MQTT代理服务器`更改为私人代理服务器，运行于`公网`，增加了`用户认证`，限制了访问用户。

功能上增加了 `定时上发消息`、`简易的LED控制指令`，可由其他客户端来控制开发板上LED的运行状态。

***

## 如何测试运行

提供个人`MQTT代理服务器`，可在公网访问，供大家测试学习使用。

### 公网 `MQTT代理服务器` 的控制台账号

【注意】具有一定的后台管理功能，可以`查看在线设备信息`、`查看订阅的主题`、`踢除设备`、`在线调试`等。仅开放两个供测试用，不要瞎搞。如登录失败，请及时联系我。

| 后台地址  | http://139.196.254.80:18083/ | 同左 |
| -------- | ------ | ------ |
| 登录用户名 | easyio | esp32 |
| 密码      | admin    | admin |


### 客户端测试账号

【注意】开放两个免费的客户端设备账号，仅供测试用，同时用一个账号不会被挤掉。购买过开发板的用户，可免费申请个人账号，将你自定义的认证方式一同发我即可。（建议使用认证方式(2)[username + password]）

提供两种认证方式：（1）[client_id + password] （2）[username + password]。【建议使用认证方式(2)。明文，为方便测试，暂未使用加密方式】

|    Host    | 139.196.254.80:1883 | 同左 |
| --------- | ------ | ------ |
| client_id | ESP32_f24a3ed0 |  |
| username  |        | easyio |
| password  | passwd | public |

域名正在备案中，备案成功后，可用 easyio-iot.com 访问。云服务器可能过两年到期后会更换，有域名就不用担心IP更换的问题。

ESP填写远端服务器地址和客户端认证消息，是通过`esp_mqtt_client_config_t`结构体进行配置。
其中`URI`的内容可指定使用的协议与端口。

如使用`MQTT`默认的`1883`端口，可以这样填写：
```c
    esp_mqtt_client_config_t mqtt_cfg = {
        //.uri = CONFIG_BROKER_URL,             // 使用menuconfig配置项的 MQTT代理URL
        .uri = "mqtt://139.196.254.80",         /*!< Complete MQTT broker URI */
        .client_id = clientID,                  /*!< default client id is ``ESP32_%CHIPID%`` where %CHIPID% are last 3 bytes of MAC address in hex format */
        .username = "easyio",                   /*!< MQTT username */
        .password = "public",                   /*!< MQTT password */
    };
```

而如果要更改为其他端口，如`1893`，可以：
```c
    esp_mqtt_client_config_t mqtt_cfg = {
        //.uri = CONFIG_BROKER_URL,             // 使用menuconfig配置项的 MQTT代理URL
        .uri = "mqtt://139.196.254.80:1893",    /*!< Complete MQTT broker URI */
        .client_id = clientID,                  /*!< default client id is ``ESP32_%CHIPID%`` where %CHIPID% are last 3 bytes of MAC address in hex format */
        .username = "easyio",                   /*!< MQTT username */
        .password = "public",                   /*!< MQTT password */
    };
```

也可以：
```c
    esp_mqtt_client_config_t mqtt_cfg = {
        //.uri = CONFIG_BROKER_URL,             // 使用menuconfig配置项的 MQTT代理URL
        .host = "139.196.254.80",               /*!< MQTT server domain (ipv4 as string) */
        .port = 1893,                           /*!< MQTT server port */
        .client_id = clientID,                  /*!< default client id is ``ESP32_%CHIPID%`` where %CHIPID% are last 3 bytes of MAC address in hex format */
        .username = "easyio",                   /*!< MQTT username */
        .password = "public",                   /*!< MQTT password */
    };
```

ESP MQTT URI 详细介绍：[ESP MQTT URI - 在线文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/protocols/mqtt.html)


***

## 操作步骤

1. 开发板的MQTT连接方式及配置项，由 `mqtt_cfg` 结构体配置。默认使用 [username + password] 的方式连接服务器。

2. 开发板用户需要用图形化配置工具，设置开发板连接的 `WIFI热点名称`、`密码`。

3. 编译、运行程序。可以在终端看到自己ESP32的MAC。（记住后4位）

4. 打开公网 `MQTT代理服务器` 的 [网页控制台](http://139.196.254.80:18083/)，点击`客户端`，可以看到有名称为 `ESP32_xxxxxxxx`(自己MAC后4位) 的客户端在线。

5. 点击自己的客户端名称，可以查看`订阅主题列表`、IP、心跳等信息。如果主动`踢除设备`，ESP32因为默认开启了自动重连，会在踢除后的预设时间内，自动重新连接代理服务器。

6. 打开 `MQTT.fx-1.7.1`，配置连接项，使用 [username + password] 的认证方式，订阅与ESP32开发板相同的主题（一共2个主题）。（也可用控制台 - 工具 - Websocket，但不推荐，容易断连）

7. 如以上操作无误，可以同时在 `VScode终端` 和 `MQTT.fx-1.7.1` 上，看到ESP32向主题 "/"+clientID+"/topic/post" 发布的计数消息，程序内默认的上传间隔为5秒。

8. 使用 `MQTT.fx-1.7.1`，向主题 "/"+clientID+"/topic/set"，发送 `led off`、`led on`，观察开发板的led运行状态。

9. 如果以上皆无误，可以看到几乎是刚发送 `led off`、`led on` 消息，开发板的led就会得到响应，体现MQTT的`即时`特性。开发板在接收到命令后会上发 "Done"。

10. 因为程序中已经对订阅的主题用`设备ID`加以区分，在有多个ESP32开发板的情况下，是可以分别控制多台设备的。

11. 开发板自带UPS后备电源，所以多台设备测试时，不必为电源线问题发愁。也因为3.3V电压域为UPS，所以不会有插接USB线导致开发板重启的问题。

【另外】之所以使用 `get_device_mac_clientid_topic(void)`，来根据设备MAC自动合成 clientID、topic_posted、topic_issued。是因为大家共用着一台公网`MQTT代理服务器`，
如果不对订阅的主题用`设备ID`加以区分，大家都订阅同一个主题，会造成信息混乱，做不到对某一设备的精准控制，不便于调试。

***

## ESP MQTT 的事件类型

- `MQTT_EVENT_CONNECTED`：客户端已成功建立与代理的连接。 客户端现在已准备好发送和接收数据。

- `MQTT_EVENT_DISCONNECTED`：客户端已中止连接，可能原因-服务器不可用或其他。

- `MQTT_EVENT_SUBSCRIBED`：代理服务器已确认客户端的订阅请求。 事件数据将包含订阅消息的消息ID。

- `MQTT_EVENT_UNSUBSCRIBED`：代理服务器已确认客户端的取消订阅请求。 事件数据将包含订阅消息的消息ID。

- `MQTT_EVENT_PUBLISHED`：代理服务器已确认客户端的发布消息。 这只会针对服务质量级别 为 Qos1 和 Qos2 发布，因为 Qos0 不使用确认。 事件数据将包含发布消息的消息ID。

- `MQTT_EVENT_DATA`：客户端已收到发布的消息。 事件数据包含：消息ID、发布消息的主题名称、接收到的数据及其长度。 对于超出内部缓冲区的数据，将发布多个 MQTT_EVENT_DATA，并更新事件数据中的 current_data_offset 和 total_data_len 以跟踪碎片化消息。

- `MQTT_EVENT_ERROR`：客户端遇到错误。 esp_mqtt_error_type_t 来自事件数据中的error_handle，可以用来进一步判断错误的类型。如WIFI网络故障、被MQTT代理服务器踢除。（ESP32的MQTT默认设置了自动重连，被剔除后会在超时时间内自动重连）

事件类型及 ESP MQTT 详细介绍：[ESP MQTT EVEVT - 在线文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/protocols/mqtt.html)


***

## 运行现象

```c

```

***

## 注意事项

* 用 VScode 编辑代码时，尽量少用 `data` 定义变量名称，会造成 VScode 卡顿。

* 公网 `MQTT代理服务器` 的 `控制台账号` 请爱惜，不要瞎搞。

* 我提供的公网 `MQTT代理服务器`，使用 `阿里云服务器` + `EMQ` 搭建，整个过程比较简单，有空的话后面会专门出一期入坑贴。
(不建议在本地搭建，费时且仅能运行于自己的局域网，宽带有公网IP或者会内网穿透另当别论。大厂云服务器新用户有优惠，一年不到100，只比.com域名略贵，主要是不用担心断电、网络问题，系统镜像可以一键替换，极致方便。满足开发测试之余，部署个人博客、个人网站也是极好的)

* ESP32连接的WIFI热点，需要有2.4GHz的频段。
