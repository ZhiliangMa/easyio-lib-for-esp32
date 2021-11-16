# 50_SNTP_DeepSleep

## 一、例程简介

NTP（Network Time Protocol--网络时间协议）

SNTP（Simple Network Time Protocol，简单网络时间协议）

NTP 算法复杂，几乎不受网络的延迟和抖动的影响，可以提供1-50 ms 精度。NTP 同时提供认证机制，安全级别很高。但是NTP 算法复杂，对系统要求较高。

SNTP（简单网络时间协议）是NTP 的简化版本，在实现时，计算时间用了简单的算法，性能较高。而精确度一般也能达到1 秒左右，基本能满足绝大多数场合的需要。

[ESP32 System Time系统时间和SNTP](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/system/system_time.html)

本Demo的主要任务`sntp_task`，搬运于IDF例程中的`SNTP`。

1. 程序运行后，会先获取当前系统时间，如果系统时间<2016年，则使用SNTP进行同步。

2. 程序会自动WIFI联网，使用SNTP协议通过NTP服务器校准系统时间，提供时区转换功能。在SNTP同步系统时间后，ESP32会进入深度睡眠10s。

3. 在深度睡眠的期间，RTC内存中的 boot_count（启动次数） 保持原样。睡眠结束后，代码重启，只有 boot_count 保持不变。

因为还开启了一个LED任务，所以在SNTP还未同步成前，LED会有一段时间的闪烁。而之后同步完成后重启，会判定为已经同步过时间，此后因ESP32处于活动时间较短，LED只会短暂的工作一下，随即进入深度睡眠。

easyio 简单的对`SNTP`例程进行了封装，封装为`sntp_systime.c.h`。使用SNTP功能时可仿照本Demo进行使用。

***

## 二、本节Demo代码的使用方法

使用图形化配置工具，配置ESP32连接的 `WIFI热点名称`、`密码`。

编译、下载、运行，观察终端的输出。

### 2.1、如何设置SNTP更新时间的方式

ESP32的SNTP有三种更新方式：（例程默认使用立即更新）
- CONFIG_SNTP_TIME_SYNC_METHOD_IMMED：收到时立即更新
- CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH：平滑更新
- CONFIG_SNTP_TIME_SYNC_METHOD_CUSTOM：自定义实现

可使用图形化配置工具修改，默认为立即更新。

### 2.2、如何修改NTP服务器和更新周期

修改`NTP服务器`地址，可到`sntp_systime.c`中的`sntp_setservername(0, "pool.ntp.org");`修改。

使用此代码将定期同步时间。时间同步周期由`CONFIG_LWIP_SNTP_UPDATE_DELAY`决定，默认值为一小时。可在项目配置中设置。

#### 2.3、如何修改时区

他俩组合使用，通过设置时区的环境变量参数，来设置时区
- setenv:设置时区的环境变量参数
- tzset:设置时区

```
    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1); // 设置时区的环境变量参数
    tzset(); // 设置时区

    setenv("TZ", "CST-8", 1);
    tzset();
```

***

## 三、运行现象

```
I (0) cpu_start: App cpu up.
I (437) heap_init: Initializing. RAM available for dynamic allocation:
I (444) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (450) heap_init: At 3FFB7880 len 00028780 (161 KiB): DRAM
I (456) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (463) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (469) heap_init: At 4009607C len 00009F84 (39 KiB): IRAM
I (475) cpu_start: Pro cpu start user code
I (494) spi_flash: detected chip: generic
I (494) spi_flash: flash io: dio
I (495) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (503) example: Boot count: 1
I (503) example: Time is not set yet. Connecting to WiFi and getting time over NTP.
I (543) wifi:wifi driver task: 3ffbfd60, prio:23, stack:6656, core=0
I (543) system_api: Base MAC address is not set
I (543) system_api: read default base MAC address from EFUSE
I (553) wifi:wifi firmware version: bb6888c
I (553) wifi:wifi certification version: v7.0
I (553) wifi:config NVS flash: enabled
I (553) wifi:config nano formating: disabled
I (563) wifi:Init data frame dynamic rx buffer num: 32
I (563) wifi:Init management frame dynamic rx buffer num: 32
I (573) wifi:Init management short buffer num: 32
I (573) wifi:Init dynamic tx buffer num: 32
I (583) wifi:Init static rx buffer size: 1600
I (583) wifi:Init static rx buffer num: 10
I (583) wifi:Init dynamic rx buffer num: 32
I (593) wifi_init: rx ba win: 6
I (593) wifi_init: tcpip mbox: 32
I (603) wifi_init: udp mbox: 6
I (603) wifi_init: tcp mbox: 6
I (603) wifi_init: tcp tx win: 5744
I (613) wifi_init: tcp rx win: 5744
I (613) wifi_init: tcp mss: 1440
I (623) wifi_init: WiFi IRAM OP enabled
I (623) wifi_init: WiFi RX IRAM OP enabled
I (633) example_connect: Connecting to CMCC-vVSQ...
I (633) phy_init: phy_version 4660,0162888,Dec 23 2020
I (743) wifi:mode : sta (08:3a:f2:4a:3f:88)
I (743) example_connect: Waiting for IP(s)
I (1353) wifi:new:<6,0>, old:<1,0>, ap:<255,255>, sta:<6,0>, prof:1
I (2123) wifi:state: init -> auth (b0)
I (2143) wifi:state: auth -> assoc (0)
I (2143) wifi:state: assoc -> run (10)
I (2143) wifi:state: run -> init (2a0)
I (2143) wifi:new:<6,0>, old:<6,0>, ap:<255,255>, sta:<6,0>, prof:1
I (2143) wifi:new:<6,0>, old:<6,0>, ap:<255,255>, sta:<6,0>, prof:1
I (2153) example_connect: Wi-Fi disconnected, trying to reconnect...
I (4213) example_connect: Wi-Fi disconnected, trying to reconnect...
I (4223) wifi:new:<6,0>, old:<6,0>, ap:<255,255>, sta:<6,0>, prof:1
I (4223) wifi:state: init -> auth (b0)
I (4223) wifi:state: auth -> assoc (0)
I (4233) wifi:state: assoc -> run (10)
I (4253) wifi:connected with CMCC-vVSQ, aid = 4, channel 6, BW20, bssid = 2c:43:be:1c:fb:ff
I (4253) wifi:security: WPA2-PSK, phy: bgn, rssi: -60
I (4263) wifi:pm start, type: 1

I (4353) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5533) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:0a3a:f2ff:fe4a:3f88, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (5533) esp_netif_handlers: example_connect: sta ip: 192.168.1.7, mask: 255.255.255.0, gw: 192.168.1.1
I (5543) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.1.7
I (5553) example_connect: Connected to example_connect: sta
I (5563) example_connect: - IPv4 address: 192.168.1.7
I (5563) example_connect: - IPv6 address: fe80:0000:0000:0000:0a3a:f2ff:fe4a:3f88, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (5573) example: Initializing SNTP
I (5583) example: Waiting for system time to be set... (1/10)
I (7593) example: Waiting for system time to be set... (2/10)
I (8963) example: Notification of a time synchronization event
I (9593) wifi:state: run -> init (0)
I (9593) wifi:pm stop, total sleep time: 3804714 us / 5330389 us

I (9593) wifi:new:<6,0>, old:<6,0>, ap:<255,255>, sta:<6,0>, prof:1
W (9593) wifi:hmac tx: stop, discard
I (9633) wifi:flush txq
I (9633) wifi:stop sw txq
I (9633) wifi:lmac stop hw txq
I (9633) wifi:Deinit lldesc rx mblock:10
I (9643) example: The current date/time in New York is: Sat Nov  6 12:10:19 2021
I (9643) example: The current date/time in Shanghai is: Sun Nov  7 00:10:19 2021
I (9643) example: Entering deep sleep for 10 seconds
```

之后ESP32会进入深度睡眠(deep sleep)。CPU，大多数RAM和所有数字时钟外围设备都将关闭，仅有RTC控制器，RTC外设(包括ULP协处理器)和RTC存储器在活动中，RTC内存的数据得以保存。
本Demo存储在RTC的数据为`boot_count 启动次数`：
```
RTC_DATA_ATTR static int boot_count = 0;
```

深度睡眠10s过后，主程序会重新运行，只有`boot_count`还保留着原来的值不变。

***

## 四、一些API的使用方法

### 4.1、设置时区
他俩组合使用，用来设置时区
- setenv:设置时区的环境变量参数
- tzset:设置时区

### 4.2、开启SNTP同步
要通过 SNTP 开始同步，只需调用以下三个功能
```
sntp_setoperatingmode(SNTP_OPMODE_POLL);
sntp_setservername(0, "pool.ntp.org");
sntp_init();
```
使用此代码将定期同步时间。时间同步周期由`CONFIG_LWIP_SNTP_UPDATE_DELAY`决定，默认值为一小时。可在项目配置中设置。

### 4.3、时间同步的状态
- sntp_get_sync_status:获取时间同步的状态。
更新完成后，状态将按SNTP_SYNC_STATUS_COMPLETED返回。之后，状态将重置为SNTP_SYNC_STATUS_RESET。如果更新操作尚未完成，状态将SNTP_SYNC_STATUS_RESET。如果选择了平稳模式，并且同步仍在继续（辅助工作），则SNTP_SYNC_STATUS_IN_PROGRESS。
返回值：
```
SNTP_SYNC_STATUS_RESET：
SNTP_SYNC_STATUS_IN_PROGRESS：时间同步正在进行中。
SNTP_SYNC_STATUS_COMPLETED：时间已同步完成。
```

### 4.4、获取当前系统时间
获取时间
```
    time(&now); // 获取当前系统时间，UTC时间Unix格式，long类型
    localtime_r(&now, &timeinfo); // 将Unix时间now，转换为当前时区的年月日时分秒，struct tm存储格式
```

- `sntp_set_time_sync_notification_cb()` - 使用这个函数来设置一个回调函数来通知时间同步过程。
- `sntp_get_sync_status()` 和 `sntp_set_sync_status()` - 获取/设置时间同步状态。
- `sntp_get_sync_mode()` 和 `sntp_set_sync_mode()` - 获取/设置系统时间的同步模式。允许的两种模式：`SNTP_SYNC_MODE_IMMED` 和`SNTP_SYNC_MODE_SMOOTH`。

`adjtime()` 是一个在“平滑”时间更新模式下自动调用的 libc 函数，但也可以从自定义时间同步代码中调用。
如果时间误差小于 35 分钟，则 `adjtime` 函数将开始平滑调整，否则返回值为 -1。


代码中NTP获得时间在`void obtain_time(void)`函数中实现。

***

## 五、注意事项

* ESP32的系统时间计数使用的是Unix时间，long类型。使用时要用`localtime_r(&now, &timeinfo);`进行转换，转换为当前时区的年月日时分秒星期这样的具象内容。

* ESP32会进入深度睡眠(deep sleep)。CPU，大多数RAM和所有数字时钟外围设备都将关闭，仅有RTC控制器，RTC外设(包括ULP协处理器)和RTC存储器在活动中，RTC内存的数据得以保存。

* `RTC_DATA_ATTR static int boot_count = 0;` 这一句就是将变量`boot_count`存入RTC内存。

* ESP32拥有多种睡眠模式，以应对不同应用场景提供更为卓越的功耗表现。本Demo仅简单演示了深度睡眠，IDF例程中有 `light sleep`、`deep sleep` 对睡眠进行了更为详细的说明。
