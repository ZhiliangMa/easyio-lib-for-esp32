
#ifndef __TOUCH_PAD_BUTTON_H
#define __TOUCH_PAD_BUTTON_H

#include "driver/touch_pad.h"
#include <stdarg.h>

#define TOUCH_THRESH_NO_USE   (0)
//#define TOUCH_THRESH_PERCENT  (80)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

//touch pad 中断触发位，记录各通道的触摸事件（0空闲，1-触摸）
extern bool s_pad_activated[TOUCH_PAD_MAX];
//touch pad 的初始感应值，用来做中断阈值的较准（注意，在初始化过程中，电极请勿触碰其他物体，以免干扰此值的正常获取）
extern uint32_t s_pad_init_val[TOUCH_PAD_MAX];


/*  ESP32 Touch_Pad 与 Pin 对照表

    +----------+--------------------+------------------+
    |          |    touch signal    |    Pin / Name    |
    +==========+====================+==================+
    |          |         T0         |      GPIO4       |
    |          +--------------------+------------------+
    |          |         T1         |      GPIO0  !    |
    |          +--------------------+------------------+
    |          |         T2         |      GPIO2       |
    |          +--------------------+------------------+
    |          |         T3         |    MTDO/GPIO15   |
    |  ESP32   +--------------------+------------------+
    |  touch   |         T4         |    MTCK/GPIO13   |
    |  sensor  +--------------------+------------------+
    |          |         T5         |    MTDI/GPIO12   |
    |          +--------------------+------------------+
    |          |         T6         |    MTMS/GPIO14   |
    |          +--------------------+------------------+
    |          |         T7         |      GPIO27      |
    |          +--------------------+------------------+
    |          |         T8         |   32K_XN/GPIO33  |
    |          +--------------------+------------------+
    |          |         T9         |   32K_XP/GPIO32  |
    +----------+--------------------+------------------+

*/

/**
 * @brief  触摸板按键检测初始化
 *      - 支持输入不定数目的通道参数，总参数数目为 2~11。
 *        例：
 *          touch_pad_button_init(1, TOUCH_PAD_NUM0);
 *          touch_pad_button_init(4, TOUCH_PAD_NUM0, TOUCH_PAD_NUM2, TOUCH_PAD_NUM3, TOUCH_PAD_NUM4);
 *      - ESP32 的T1为GPIO0，一般有电阻下拉到GND做BOOT模式选择，导致触摸失效，所以尽量不要用T1做 touch pad 功能
 * 
 * @param  tp_num   总触摸按键 的数量
 * @param  (...)    触摸通道列表。支持不定数目参数，数目为 1~10。值为 TOUCH_PAD_NUM0, TOUCH_PAD_NUM1... TOUCH_PAD_NUM9。
 * 
 * @return
 *      - none
 * 
 */
void touch_pad_button_init(int tp_num, ...);

#endif
