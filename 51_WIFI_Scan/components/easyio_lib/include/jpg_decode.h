#ifndef __JPG_DECODE_H__
#define __JPG_DECODE_H__

#include <stdint.h>
#include "esp_err.h"


/**
 * 为了便于以后重复解码，解码过程分为三步(三个函数)：1、申请输出像素内存和TJpgDec解码器内存；2、解码；3、释放内存
 * 因为大多数应用场景是重复解码，所以一般用不到 释放内存
 * 重复解码的话，需要在申请内存时，就要按照可能解码的最大像素值来申请
 */

/**
 * @brief  申请jpg解码器、和输出到LCD的RGB565像素 需要的内存空间（此为TJpgDec解码器的预处理，仅需调用一次。重复调用可能会因为申请的空间超出ESP32可用范围，导致错误。）
 *      - 注意：buf_height、buf_width 决定了申请内存的多少，需要填写图片有可能解码得到的最大像素值，buf_height、buf_width 需要大于等于图片解码后的像素个数。
 *      - 此函数为 三步解码过程中的第一步（1、申请输出像素内存和TJpgDec解码器内存）
 *      - 注意：如果像素宽高比较大，pixels最后申请的内存也会变大。ESP32在不用PSRAM的情况下，可用申请空间不到200KB。320x240的屏幕仅能调用一次，再次调用会申请内存失败。
 * 
 * @param  pixels 存储着解码后的RGB565像素内容的指针
 * @param  buf_height jpg图片高度的像素数目（可>实际图片的像素大小，按照可能输入jpg图片的最大高度填写）
 * @param  buf_width jpg图片宽度的像素数目（可>实际图片的像素大小，按照可能输入jpg图片的最大宽度填写）
 * 
 * @return
 *     - esp_err_t
 */
esp_err_t jpg_decode_request_ram(uint16_t ***pixels, uint16_t buf_height, uint16_t buf_width);

/**
 * @brief  解码jpg图片，输出为RGB565的像素格式
 *      - 调用TJpgDec的解码算法，输出为RGB565的像素格式
 *      - 此函数为 三步解码过程中的第二步（2、解码；）
 * 
 * @param  jpg_img 输入jpg图片的文件指针
 * @param  pixels 输出解码后RGB565像素内容的指针
 * @param  scale 解码图像输出的比例 1/1,1/2,1/4,1/8（0~3）
 * 
 * @return
 *     - esp_err_t
 */
esp_err_t jpg_decode(const unsigned char *jpg_img, uint16_t ***pixels, uint8_t scale);


#endif
