# 18_RMT_IR_RX

## 例程简介

介绍ESP32的 `rmt_ir` 的使用，进行`38KHz红外调制接收管`信号的解析。

使用 `FreeRTOS` 的 `Task`，创建三个任务：

1. **led_task** ，控制LED闪烁

2. **rmt_ir_rx_task** ，初始化`rmt模块`，接收并解析出，来自`38KHz红外调制接收管`的信号。使用`ESP-IDF`提供的红外解析库 `infrared_tools`进行命令帧解析（NEC格式），串口调试输出 `命令帧中逻辑位的高低电平持续时间（单位us）`、`红外命令帧的内容`

使用 `rmt_ir.c.h` 驱动模块，来对ESP32的 `rmt` 进行配置，并解析红外遥控的命令帧数据。


## 运行现象

* LED闪烁。
 
* 准备红外遥控器，对准开发板按按键，可从串口调试窗口输出 `逻辑位数量`、`命令帧中逻辑位的高低电平持续时间（单位us）`、`红外命令帧的内容`。

终端输出内容如下：
```
Bit num: 34
RAM:
1:9214  0:4516
1:600   0:573
1:626   0:548
1:627   0:547
1:597   0:575
1:599   0:573
1:598   0:575
1:622   0:552
1:620   0:553
1:597   0:1661
1:596   0:1660
1:624   0:1635
1:597   0:1662
1:596   0:1661
1:597   0:1661
1:597   0:1662
1:596   0:1662
1:595   0:1663
1:595   0:578
1:594   0:1664
1:595   0:579
1:593   0:1664
1:594   0:580
1:594   0:579
1:594   0:580
1:593   0:580
1:592   0:1667
1:592   0:581
1:592   0:1667
1:590   0:583
1:590   0:1667
1:592   0:1666
1:592   0:1667
1:590   0:0
I (62823) example: Scan Code  --- addr: 0xff00 cmd: 0xea15
Bit num: 2
RAM:
1:9202  0:2251
1:590   0:0
I (62843) example: Scan Code (repeat) --- addr: 0xff00 cmd: 0xea15
Bit num: 2
RAM:
1:9208  0:2247
1:592   0:0
I (62953) example: Scan Code (repeat) --- addr: 0xff00 cmd: 0xea15
Bit num: 2
RAM:
1:9207  0:2245
1:621   0:0
I (63053) example: Scan Code (repeat) --- addr: 0xff00 cmd: 0xea15
```

其中`Bit num: 34`的段为命令帧，地址码+命令码。

`Bit num: 2`的段为重复码。只要按键保持按下状态，就会每110毫秒发送一次重复码。 该重复码的组成为 `9ms的AGC脉冲` + `2.25ms的空闲间隔` + `560µs的脉冲`。


## 学习内容

1. ESP32的`RMT`配置及用法。

2. 了解38KHz红外遥控的编码格式、NEC协议。

38K红外及NEC编码只是，详见我的博客：

[NEC红外协议编码](https://blog.csdn.net/Mark_md/article/details/115053032?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163048946816780357252315%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163048946816780357252315&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29-1-115053032.pc_v2_rank_blog_default&utm_term=nec&spm=1018.2226.3001.4450)

[38K红外硬件设计](https://blog.csdn.net/Mark_md/article/details/115050817?spm=1001.2014.3001.5501)


## 关键函数

```c
// 配置RMT-RX，接收38KHz红外输入，另外分配环形缓冲区
void rmt_ir_rx_init(gpio_num_t gpio_num, rmt_channel_t channel_id, RingbufHandle_t* ring_buf, size_t rx_buf_size);

// 调试输出RMT RAM接收到的内容，仅调试用
void rmt_print_ram(rmt_item32_t *items, uint32_t numItems);

// 注册NEC红外编码的解析
ir_parser_rmt_new_nec(&ir_parser_config);
```


### 注意事项

* `ESP-IDF`的Demo中提供了红外解析库 `infrared_tools`，但只能解析标准NEC/RC5，不能解析空调等非标红外信号。好在使用起来够简单，可以直接调用。`easyio`将`infrared_tools`直接拿了过来。

* 一般要另外修改时间容差，位于 `"ir_tools.h"` 文件中的 `margin_us`，单位是us，默认容差为`260us`。实际对一些NEC格式的遥控器进行解析发现，有的遥控器的误差已经到了`200us`，如果超出预设范围则会导致不能被正常解析。所以如遇到因遥控器误差引起的解析失败，请修改 `margin_us` 的值。
```c
#define IR_PARSER_DEFAULT_CONFIG(dev) \
    {                                 \
        .dev_hdl = dev,               \
        .flags = 0,                   \
        .margin_us = 260,             \
    }
```

* 家用的空调遥控等，编码方式大多为非标，字节都不一定能对齐，解析需要先搜索该品牌型号的红外编码资料。
