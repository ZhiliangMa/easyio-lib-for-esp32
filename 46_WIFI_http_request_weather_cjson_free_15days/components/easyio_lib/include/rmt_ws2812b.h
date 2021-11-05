#ifndef __RMT_WS2812B_H__
#define __RMT_WS2812B_H__

#include "driver/rmt.h"
#include "led_strip.h"
#include "esp_log.h"

#define WS2812B_TX_CHANNEL    RMT_CHANNEL_2
#define WS2812B_TX_GPIO       33
#define WS2812B_LED_NUMBER    24 // ws2812b灯珠个数

//灯带驱动的对象，外部可直接调用
extern led_strip_t *strip;

/**
 * @brief  配置RMT-TX，并安装 ws2812B驱动
 * 
 * @param  gpio_num RMT-TX驱动WS2812B的管脚号。除了几个仅能做输入的其他任意GPIO
 * @param  channel_id 使用的RMT单元号。RMT_CHANNEL_0~7
 * @param  led_num ws2818B led灯珠的数量
 * 
 * @return
 *     - none
 * 
 */
void rmt_ws2812b_tx_init(gpio_num_t gpio_num, rmt_channel_t channel_id, uint16_t led_num);

/**
 * @brief 将HSV颜色空间转换为RGB颜色空间
 *      - 因为HSV使用起来更加直观、方便，所以代码逻辑部分使用HSV。但WS2812B RGB-LED灯珠的驱动使用的是RGB，所以需要转换。
 * 
 * @param  h HSV颜色空间的H：色调。单位°，范围0~360。（Hue 调整颜色，0°-红色，120°-绿色，240°-蓝色，以此类推）
 * @param  s HSV颜色空间的S：饱和度。单位%，范围0~100。（Saturation 饱和度高，颜色深而艳；饱和度低，颜色浅而发白）
 * @param  v HSV颜色空间的V：明度。单位%，范围0~100。（Value 控制明暗，明度越高亮度越亮，越低亮度越低）
 * @param  r RGB-R值的指针
 * @param  g RGB-G值的指针
 * @param  b RGB-B值的指针
 *
 * Wiki: https://en.wikipedia.org/wiki/HSL_and_HSV
 *
 */
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b);


#endif
