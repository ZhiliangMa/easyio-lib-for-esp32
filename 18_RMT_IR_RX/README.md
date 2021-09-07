# 18_RMT_IR_RX

## 例程简介

介绍ESP32的 `rmt_ir` 的使用，进行`38KHz红外调制接收管`信号的解析。

使用 `FreeRTOS` 的 `Task`，创建三个任务：

1. **led_task** ，控制LED闪烁

2. **rmt_ir_rx_task** ，初始化`rmt模块`，接收并解析出，来自`38KHz红外调制接收管`的信号。使用`ESP-IDF`提供的红外解析库 `infrared_tools`进行命令帧解析，串口调试输出 `命令帧中逻辑位的高低电平持续时间（单位us）`、`红外命令帧的内容`

使用 `rmt_ir.c.h` 驱动模块，来对ESP32的 `rmt` 进行配置，并解析红外遥控的命令帧数据。


## 运行现象

* LED闪烁。
 
* 准备红外遥控器，对准开发板按按键，可从串口调试窗口输出 `命令帧中逻辑位的高低电平持续时间（单位us）`、`红外命令帧的内容`。


## 学习内容

1. ESP32的`RMT`配置及用法。

2. 了解38KHz红外遥控的编码格式、NEC协议。

NEC红外协议编码：https://blog.csdn.net/Mark_md/article/details/115053032?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163048946816780357252315%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163048946816780357252315&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29-1-115053032.pc_v2_rank_blog_default&utm_term=nec&spm=1018.2226.3001.4450


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

* 一般要另外修改时间容差，位于 `"ir_tools.h"` 文件中的 `margin_us`，单位是us，默认容差为`200us`。实际对一些NEC格式的遥控器进行解析发现，有的遥控器的误差已经到了`170us`，如果质量再差一点的很可能就超出预先设定的`200us`容差了，导致不能被正常解析。所以如遇到因遥控器误差引起的解析失败，请修改 `margin_us` 的值。
```c
#define IR_PARSER_DEFAULT_CONFIG(dev) \
    {                                 \
        .dev_hdl = dev,               \
        .flags = 0,                   \
        .margin_us = 200,             \
    }
```

* 家用的空调遥控等，大多为非标，字节都不一定能对齐，解析需要先搜索品牌型号的红外编码资料。
