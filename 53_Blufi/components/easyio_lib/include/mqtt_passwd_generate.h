#ifndef __MQTT_PASSWD_GENERATE_H__
#define __MQTT_PASSWD_GENERATE_H__

#include "esp_log.h"
#include "mbedtls_encrypt.h"


// mqPwd调试信息输出开关，通过串口调试输出 mqtt的认证密码 信息（0关闭，1开启）
#define mqPwd_Dlog    1

/**
 * @brief  输入设备证书（三元组）和接入的服务器地址，函数会自动计算并输出 MQTT认证所需的 mqttClientId、mqttUsername、mqttPassword、brokerUrl，
 *         以及基本的 “属性上报”/“设置下发” 的主题名称 topic_post、topic_set
 *         - 阿里云物联网平台 mqttClientId、mqttUsername、mqttPassword 的合成方法见在线文档：https://help.aliyun.com/document_detail/73742.html?spm=a2c4g.11186623.6.614.c92e3d45d80aqG
 *         - 阿里云物联网频台的 属性上报/设置事件 的主题格式：https://help.aliyun.com/document_detail/89301.htm?spm=a2c4g.11186623.0.0.221f4ec917XuMh#section-g4j-5zg-12b
 *         - 这里使用的是 “一机一密、一型一密” 的预注册认证方式，适合个人开发及测试使用，不适合量产。量产大多会使用 “一型一密” 的动态注册
 * 
 * @param  productKey   填写设备证书中的 productKey
 * @param  deviceName   填写设备证书中的 deviceName
 * @param  deviceSecret 填写设备证书中的 deviceSecret
 * @param  regionId     填写接入的服务器地址
 * @param  mqttClientId mqttClientId 输出，指针指向的字符串长度建议>60
 * @param  mqttUsername mqttUsername 输出，指针指向的字符串长度建议>60
 * @param  mqttPassword mqttPassword 输出，指针指向的字符串长度建议>50
 * @param  brokerUrl    mqtt 接入点域名输出，指针指向的字符串长度建议>60
 * @param  topic_post   输出 客户端向服务器上报消息的主题，指针指向的字符串长度建议>60
 * @param  topic_set    输出 服务器向客户端下发消息的主题，指针指向的字符串长度建议>60
 * 
 * @return
 *     - none
 */
void aliyun_iot_device_clientKey_generate(char *productKey, char *deviceName, char *deviceSecret, char *regionId,
                                          char *mqttClientId, char *mqttUsername, char *mqttPassword, char *brokerUrl, char *topic_post, char *topic_set);


#endif
