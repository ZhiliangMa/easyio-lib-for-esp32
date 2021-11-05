
#ifndef __KEY_H
#define __KEY_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "gpioX.h"
#include <stdarg.h>
#include "esp_log.h"

//MCU的GPIO的最大序号，或者说使用的KEY的GPIO序号最大多大，这个必须大于。当然越大使用的栈空间就越大。
#define MAX_GPIO    36

//滤除按键抖动，的阈值。8000 = 8ms
#define Jitter      8*1000
//短按的阈值
#define ShortPress  8*1000
//长按的阈值
#define LongPress   1000*1000

//key调试信息输出开关，通过串口调试输出按键信息（0关闭，1开启）
#define key_Dlog    1

//按键触发事件，的队列句柄（过滤过按键抖动，存储着短按、长按信息事件）供外部应用调用
// key_evt_queue队列接收到的消息类型为uint32_t，其中前两个字节为 按键类型（1短按，2长按），后两个为按键gpio序号，使用时需要拆分。
extern xQueueHandle key_evt_queue;

/**
 * @brief  按键检测任务
 *     - 需要FreeRTOS单独创建一个任务，单独去运行这个按键检测任务。栈空间大小与 MAX_GPIO有关，建议起步4096
 *     - 用户需要另外创建队列接收任务，接收 key_evt_queue 队列的按键事件。
 *     - key_evt_queue队列接收到的消息类型为uint32_t，其中前两个字节为 按键类型（1短按，2长按），后两个为按键gpio序号，使用时需要拆分。
 *     - 如需关闭key串口调试开关，请将 key.h 中的 key_Dlog 设置为0
 *     - 对于本代码，电路使用按键直连GND。驱动中ESP32默认开启GPIO的上拉电阻。
 * 
 *     - 支持输入不定数目的GPIO通道参数，以便用一句函数实现多按键初始化。
 *     - 例：key_scan(1, KEY);
 *     - 例：key_scan(3, 12, 13, 14);
 * 
 * @param  key_num   总按键 的数量
 * @param  (...)    按键GPIO序号列表。支持不定数目参数，数目为 0~MAX_GPIO。值为 0, 1, 2... MAX_GPIO。
 * 
 * @return
 *     - none
 * 
 */
void key_scan(int key_num, ...);

#endif
