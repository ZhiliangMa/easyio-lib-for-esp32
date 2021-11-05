#include "jpg_decode.h"

#include "tjpgd.h"
#include "esp_log.h"
#include <string.h>


const char *TAG = "TJpgDec";

// jpeg解码器的工作空间大小。（TJpgDec库的RAM占用仅为3KB，分配多了也是空闲着，只要大于1024x3即可）
#define WORKSZ 3100

/**
 * 为了便于以后重复解码，解码过程分为三步(三个函数)：1、申请输出像素内存和TJpgDec解码器内存；2、解码；(3、释放内存)
 * 因为大多数应用场景是重复解码，所以一般用不到 释放内存
 * 重复解码的话，需要在申请内存时，就要按照可能解码的最大像素值，来申请解码后的RGB565像素格式的输出空间
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
char *work = NULL; // TJpgDec 工作区，这个为全局，以便产生错误时释放内存调用。
uint16_t buf_height_size=0;
esp_err_t jpg_decode_request_ram(uint16_t ***pixels, uint16_t buf_height, uint16_t buf_width)
{
    *pixels = NULL;
    esp_err_t ret = ESP_OK;
    buf_height_size = buf_height; // 记录 *pixels 指针数量，以便错误时释放

    // 为解码输出的RGB565像素内容申请内存。每行都是一个 16-bit的 buf_width 像素数组; * pixels数组包含指向这些行的指针。
    // buf_height、buf_width 需要大于等于图片解码后的像素。
    // *pixels 存储着每行像素的指针。
    *pixels = calloc(buf_height, sizeof(uint16_t *));
    if (*pixels == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for lines");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }
    for (int i = 0; i < buf_height; i++) {
        (*pixels)[i] = malloc(buf_width * sizeof(uint16_t));
        if ((*pixels)[i] == NULL) {
            ESP_LOGE(TAG, "Error allocating memory for line %d", i);
            ret = ESP_ERR_NO_MEM;
            goto err;
        }
    }

    // 为jpeg解码器分配工作空间。
    work = calloc(WORKSZ, 1);
    if (work == NULL) {
        ESP_LOGE(TAG, "Cannot allocate workspace");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }
    ESP_LOGI(TAG, "Allocate pixels output memory success!");
    ESP_LOGI(TAG, "Allocate TJpgDec workspace success!");
    // 申请内存空间完成， 返回信息
    // !!!注意，这里没有释放 TJpgDec 的工作区内存。也没有释放 解码出的像素 *pixels 内存，以便下次重复解码使用。
    return ret;

err:
    // 出问题了！ 清理并退出，释放分配的所有内存。
    if (*pixels != NULL) {
        for (int i = 0; i < buf_height; i++) {
            free((*pixels)[i]);
        }
        free(*pixels);
    }
    free(work);
    return ret;
}

//Data that is passed from the decoder function to the infunc/outfunc functions.
typedef struct {
    const unsigned char *inData; //Pointer to jpeg data
    uint16_t inPos;              //Current position in jpeg data
    uint16_t **outData;          //Array of IMAGE_H pointers to arrays of IMAGE_W 16-bit pixel values
    int outW;                    //Width of the resulting file
    int outH;                    //Height of the resulting file
} JpegDev;

// Jpeg文件的输入函数（片内flash直接用指针。片外SD需要先将.jpg图片读到内存）
// JpegDev解码器的输入函数。 仅从JpegDev结构的inData字段返回字节。
static uint16_t infunc(JDEC *decoder, uint8_t *buf, uint16_t len)
{
    //Read bytes from input file
    JpegDev *jd = (JpegDev *)decoder->device;
    if (buf != NULL) {
        memcpy(buf, jd->inData + jd->inPos, len);
    }
    jd->inPos += len;
    return len;
}

// 输出函数。 （暂未用到）（配合TJpgDec使用RGB888输出格式。另外计算RGB888转565。效率不如直接用TJpgDec的RGB565输出快）
// 将来自解码器的RGB888数据重新编码为 big-endian RGB565，并将其存储在JpegDev结构的outData数组中。
static uint16_t outfunc_rgb888_to_rgb565(JDEC *decoder, void *bitmap, JRECT *rect)
{
    JpegDev *jd = (JpegDev *)decoder->device;
    uint8_t *in = (uint8_t *)bitmap;
    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++) {
            //We need to convert the 3 bytes in `in` to a rgb565 value.
            uint16_t v = 0;
            v |= ((in[0] >> 3) << 11);
            v |= ((in[1] >> 2) << 5);
            v |= ((in[2] >> 3) << 0);
            //The LCD wants the 16-bit value in big-endian, so swap bytes
            v = (v >> 8) | (v << 8);
            jd->outData[y][x] = v;
            in += 3;
        }
    }
    return 1;
}

// 输出函数。 （配合TJpgDec使用RGB565输出格式。另外进行大小端转换）
// 直接使用解码器的 RGB565 输出，并将其存储在JpegDev结构的outData数组中。
static uint16_t outfunc(JDEC *decoder, void *bitmap, JRECT *rect)
{
    JpegDev *jd = (JpegDev *)decoder->device;
    uint8_t *in = (uint8_t *)bitmap;
    uint16_t v = 0;
    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++) {
            // TJpgDec 使用RGB565输出时，由于与ESP32大小端不一致，
            // 不能使用 uint16_t *in = (uint16_t *)bitmap; jd->outData[y][x] = *in; in ++; 的写法
            v = ((uint16_t)*in<<8) | *(in+1);
            jd->outData[y][x] = v;
            in += 2;
        }
    }
    return 1;
}

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
esp_err_t jpg_decode(const unsigned char *jpg_img, uint16_t ***pixels, uint8_t scale)
{
    int r;
    JDEC decoder;
    JpegDev jd;
    esp_err_t ret = ESP_OK;

    // 填充JpegDev结构的字段。
    // image_jpg_start 为 jpg原图。pixels 为输出解码后RGB565像素内容的指针。
    jd.inData = jpg_img;
    jd.inPos = 0;
    jd.outData = *pixels;

    // 准备并解码jpeg。（使用 TJpgDec库，进行jpg解码）
    r = jd_prepare(&decoder, infunc, work, WORKSZ, (void *)&jd);
    if (r != JDR_OK) {
        ESP_LOGE(TAG, "Image decoder: jd_prepare failed (%d)", r);
        ret = ESP_ERR_NOT_SUPPORTED;
        goto err;
    }
    // 输出jpg图片的像素尺寸。宽、高。
    ESP_LOGI(TAG, "Image size: %d x %d", decoder.width, decoder.height);
    //ESP_LOGI(TAG, "Image size: %d x %d", jd.outW, jd.outH);
    // 开始解码
    r = jd_decomp(&decoder, outfunc, scale);
    if (r != JDR_OK && r != JDR_FMT1) {
        ESP_LOGE(TAG, "Image decoder: jd_decode failed (%d)", r);
        ret = ESP_ERR_NOT_SUPPORTED;
        goto err;
    }
    // 解码完成， 返回信息
    // !!!注意，这里没有释放 TJpgDec 的工作区内存。也没有释放 输出解码后RGB565像素内容 *pixels 内存，以便下次重复解码使用。
    return ret;

err:
    // 解码错误的原因，大概率是因为引入了QQ截图的.jpg图片。在此为了任务考虑，暂不做处理。
    // 像下面代码那样，出错后释放TJpgDec工作空间和RGB565像素输出内存，会导致下一次调用出错，进而导致重启。
    /*// 出问题了！ 清理并退出，释放分配的所有内存。
    if (*pixels != NULL) {
        for (int i = 0; i < buf_height_size; i++) {
            free((*pixels)[i]);
        }
        free(*pixels);
    }
    free(work);*/
    return ret;
}
