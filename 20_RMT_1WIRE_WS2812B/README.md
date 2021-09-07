# 20_RMT_1WIRE_WS2812B

## 例程简介

ESP32的 `RMT` 硬件，由于其独特的内存机制，使得其非常适合`单总线数据的接收和发送`。上两个Demo使用的`红外发射和接收`只是一个非常典型的应用，还可以使用`RMT`去做一些有趣的事情，例如`单总线温湿度传感器`、`WS2812B单总线RGB-LED灯带`。

使用 `FreeRTOS` 的 `Task`，创建三个任务：

1. **led_task** ，控制LED闪烁

2. **rmt_ws2812b_task** ，初始化`rmt模块`，配置`RMT-TX`，并注册安装 `ws2812B驱动`，将HSV颜色空间转换为RGB，控制LED灯带的五彩流光效果。

Demo中设置的`WS2812B`灯带长度为`24灯珠`，实际最大长度可非常长，取决于`RMT的内存大小`。


使用 `rmt_ir.c.h` 驱动模块，来对ESP32的 `rmt` 进行配置。发射38KHz红外输出，使用ESP-IDF自带的红外编码库，发送单键码和重复码。解析红外遥控的命令帧数据。


## 运行现象

* LED闪烁。
 
* `WS2812B`灯带呈现五彩流光的效果，Demo中演示的长度为`24灯珠`。


## 学习内容

1. ESP32的`RMT`配置及用法。

2. `WS2812B`的单总线通信协议格式。

3. `HSV`颜色空间的概念、`HSV转RGB`代码。

HSV颜色空间转RGB(C语言代码)，ESP32使用RMT驱动WS2812B，七彩流光灯光效果：https://blog.csdn.net/Mark_md/article/details/115132435?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163049290916780264078694%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163049290916780264078694&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29-1-115132435.pc_v2_rank_blog_default&utm_term=hsv&spm=1018.2226.3001.4450


## 关键函数

```c
// 配置RMT-TX，并安装 ws2812B驱动
void rmt_ws2812b_tx_init(gpio_num_t gpio_num, rmt_channel_t channel_id, uint16_t led_num);

// 将HSV颜色空间转换为RGB颜色空间
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b);

// 将RGB值写入到一个led灯珠的驱动缓存（注意：此函数一次只能编辑一个灯珠的RGB值）
strip->set_pixel(strip, i, red, green, blue);

// 将整条灯带的RGB缓存值，通过RMT刷新到LED显示。超时时间100ms
strip->refresh(strip, 100);
```


## 注意事项

* `WS2812B` 在接通5V电源时，DATA引脚上会有电压，与ESP32连接时请注意`电平兼容`和`空闲电压`。

* Demo中设置的`WS2812B`灯带长度为`24灯珠`，实际最大长度可非常长，取决于`RMT的内存大小`。

* 驱动`WS2812B`灯带时，实际灯珠长度可比Demo中设定的少。例如此Demo中设置灯珠默认为24个，实际使用8个或者更少的灯珠也是能点亮的，并显示原来长度的前xx个灯珠的信息。

* 五彩流光效果非常适合用HSV颜色空间。使用起来更加直观、方便，所以代码逻辑部分使用HSV。但WS2812B RGB-LED灯珠的驱动使用的是RGB，所以需要转换。

* `Demo`中`LED`的`明度`已经被调低为10。因为电脑USB未必能输出足够的电流，会导致ESP32开发板在带LED灯带后，不能正常下载代码。如实际使用中觉得亮度不够，可将其改回100，并务必请保持足够大的电流供给。
