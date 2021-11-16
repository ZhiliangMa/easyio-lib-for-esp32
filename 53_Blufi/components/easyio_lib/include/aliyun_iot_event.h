#ifndef __ALIYUN_IOT_EVENT_H__
#define __ALIYUN_IOT_EVENT_H__

#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "esp_log.h"


/**
 * @brief  合成上报阿里云物联网平台的 温湿度传感器信息的json字段
 *         - 注意：使用此函数过后，请务必释放返回指针的内存，以避免重复调用造成内存溢出
 * 
 * @param  temperature    温度，double，单位℃，范围 -40~80
 * @param  humidity       湿度，int，单位%，范围 0~100
 * 
 * @return
 *     - json字符串指针。【注意】：使用此函数过后，请务必释放返回指针的内存，以避免重复调用造成内存溢出
 */
char *aliyun_iot_Temperature_Humidity_json_splice(double temperature, int humidity);

/**
 * @brief  合成上报阿里云物联网平台的 温湿度传感器、光照度、电池电压信息的json字段
 *         - 注意：使用此函数过后，请务必释放返回指针的内存，以避免重复调用造成内存溢出
 * 
 * @param  temperature    温度，double，单位℃，范围 -40~80
 * @param  humidity       湿度，int，单位%，范围 0~100
 * @param  mlux           光照度，int，范围 0~65535
 * @param  Voltage        电池电压，double，单位V，范围 0~5
 * 
 * @return
 *     - json字符串指针。【注意】：使用此函数过后，请务必释放返回指针的内存，以避免重复调用造成内存溢出
 */
char *aliyun_iot_Temperature_Humidity_mlux_Voltage_json_splice(double temperature, int humidity, int mlux, double Voltage);

/**
 * @brief  合成上报阿里云物联网平台的 温湿度传感器、光照度、电池电压、状态灯状态信息的json字段
 *         - 注意：使用此函数过后，请务必释放返回指针的内存，以避免重复调用造成内存溢出
 * 
 * @param  temperature    温度，double，单位℃，范围 -40~80
 * @param  humidity       湿度，int，单位%，范围 0~100
 * @param  mlux           光照度，int，范围 0~65535
 * @param  Voltage        电池电压，double，单位V，范围 0~5
 * @param  statusLight    状态灯运行状态。0-停止闪烁；1-闪烁。
 * 
 * @return
 *     - json字符串指针。【注意】：使用此函数过后，请务必释放返回指针的内存，以避免重复调用造成内存溢出
 */
char *aliyun_iot_Temperature_Humidity_mlux_Voltage_statusLight_json_splice(double temperature, int humidity, int mlux, double Voltage, int statusLight);

/**
 * @brief  cjson解析阿里云物联网平台的 `状态灯开关` 控制消息
 * 
 * @param  rdata  阿里云物联网平台MQTT收到的控制json报文
 * 
 * @return
 *     - int。状态灯的状态。0/1。错误时返回-1。
 */
int cjson_parse_aliyun_iot_StatusLightSwitch(char *rdata);

#endif
