#ifndef __HTTP_GET_WEATHER_H__
#define __HTTP_GET_WEATHER_H__

#include <stdio.h>
#include "cJSON.h"


/**
 * @brief  cjson解析心知天气报文
 * 
 * @param  rdata  心知天气http返回的JSON字符串指针
 * 
 * @return
 *     - none
 */
void cjson_parse_xinzhi_weather(char *rdata);

/**
 * @brief  cjson解析sojson天气报文
 * 
 * @param  rdata  sojson天气http返回的JSON字符串指针
 * 
 * @return
 *     - none
 */
void cjson_parse_sojson_weather(char *rdata);


#endif
