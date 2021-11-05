# 19_RMT_IR_RX_TX

## 例程简介

介绍ESP32的 `rmt_ir` 的使用，进行`38KHz红外调制接收管`信号的解析。

使用 `FreeRTOS` 的 `Task`，创建三个任务：

1. **led_task** ，控制LED闪烁

2. **rmt_ir_rx_task** ，初始化`rmt模块`，接收并解析出，来自`38KHz红外调制接收管`的信号。使用`ESP-IDF`提供的红外解析库 `infrared_tools`进行命令帧解析，串口调试输出 `命令帧中逻辑位的高低电平持续时间（单位us）`、`红外命令帧的内容`

3. **rmt_ir_tx_task** ，配置RMT-TX，发射38KHz红外输出，使用ESP-IDF自带的红外编码库，发送单键码和重复码。

在开发板上，红外发射管和接收管是并排放置，发射了什么内容，红外接收管基本都能原样接收。并用串口调试输出命令帧信息。

【测试自发自收时，建议在开发板上盖一张白纸，可以避免因反射面太远，而造成收发不一致的现象】

如果将`"rmt_ir.h"`中的`RMT1_TX_GPIO`更改为其他不是`红外LED`的IO，则无接收命令帧的输出。(由于发射和接收靠的太近，单纯靠遮挡去阻拦红外信号几乎没用，还是能接收到命令帧。可以用更改IO的方法验证确实是红外LED在发射数据)

使用 `rmt_ir.c.h` 驱动模块，来对ESP32的 `rmt` 进行配置，发射38KHz红外载波信号。使用ESP-IDF自带的红外编码库，发送单键码和重复码。同时解析接收到的红外命令帧数据。


## 运行现象

* LED闪烁。
 
* 可从串口调试窗口输出 `命令帧中逻辑位的高低电平持续时间（单位us）`、`红外命令帧的内容`。

* 代码中的动作为，以2s为间隔，向地址 `0x10`，发送 从`0x20`递增的命令。


## 学习内容

1. ESP32的`RMT`配置及用法。

2. 了解38KHz红外遥控的编码格式、NEC协议。

NEC红外协议编码：https://blog.csdn.net/Mark_md/article/details/115053032?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163048946816780357252315%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163048946816780357252315&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29-1-115053032.pc_v2_rank_blog_default&utm_term=nec&spm=1018.2226.3001.4450


## 关键函数

```c
// 配置RMT-TX，发射38KHz红外输出
void rmt_ir_tx_init(gpio_num_t gpio_num, rmt_channel_t channel_id);

// 注册NEC红外编码的构建
ir_builder_rmt_new_nec(&ir_builder_config);
```


### 注意事项

* 测试自发自收时，建议在开发板上盖一张白纸，可以避免因反射面太远，而造成收发不一致的现象。

* 与`17_RMT_IR`的注意事项一样。

* ESP32的GPIO，直接`推挽输出`能驱动的`红外LED`的电流有限。驱动大功率红外LED，要增加`MOS`或三极管来扩流，开发板的电路中就是使用`nMOS`来驱动20mA红外LED。
