#include "mqtt_passwd_generate.h"

static const char *TAG = "mqPwd";

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
                                          char *mqttClientId, char *mqttUsername, char *mqttPassword, char *brokerUrl, char *topic_post, char *topic_set)
{
    // mqttClientId[60] = ClientID + "|securemode=3,signmethod=hmacsha1|"
    // 这里的 ClientID可用户自定义，建议使用 deviceName + MAC，方便区分不同设备。也可直接填写deviceName
    // mqttClientId[60] = deviceName + "|securemode=3,signmethod=hmacsha1|"
    sprintf(mqttClientId, "%s|securemode=3,signmethod=hmacsha1|", deviceName);
    
    // mqttUsername = deviceName + "&" + productKey
    sprintf(mqttUsername, "%s&%s", deviceName, productKey);
    
    // password的明文：content = "clientId" + deviceName + "deviceName" + deviceName + "productKey" + productKey
    char content[100] = {0};
    sprintf(content, "clientId%sdeviceName%sproductKey%s", deviceName, deviceName, productKey);

    // mqttPassword合成。对content进行 HmacSHA-1加密，并以16进制字符串显示
    // 阿里云MQTT消息队列用 Base64编码，物联网平台不用
    char cipher[30] = {0}; // HmacSHA-1加密后的密文
    char cipherHex[50] = {0}; // HmacSHA-1加密后的密文，以16进制字符串显示
    char cipherBase64[30] = {0}; // HmacSHA-1加密后的密文，进行Base64编码
    hmac_sha_base64_encrypt(MBEDTLS_MD_SHA1, (uint8_t*)content, (uint8_t*)deviceSecret, (uint8_t*)cipher, (uint8_t*)cipherHex, (uint8_t*)cipherBase64); // HmacSHA-1加密、Base64编码
    strcpy(mqttPassword, cipherHex);

    // MQTT 接入点域名
    // brokerUrl = productKey + ".iot-as-mqtt." + regionId + ".aliyuncs.com"
    sprintf(brokerUrl, "%s.iot-as-mqtt.%s.aliyuncs.com", productKey, regionId);

    // Topic，post，客户端向服务器上报消息
    // topic_post = "/sys/" + productKey + "/" + deviceName + "/thing/event/property/post"
    sprintf(topic_post, "/sys/%s/%s/thing/event/property/post", productKey, deviceName);
    // Topic，set，服务器向客户端下发消息
    // topic_set = "/sys/" + productKey + "/" + deviceName + "/thing/service/property/set"
    sprintf(topic_set, "/sys/%s/%s/thing/service/property/set", productKey, deviceName);

    #if mqPwd_Dlog
    // 调试输出。可通过更改.h文件中的 mqPwd_Dlog 宏定义，来选择调试信息开关（0关闭，1开启）
    ESP_LOGW(TAG, "mqttClientId: %s\n", mqttClientId);
    ESP_LOGW(TAG, "mqttUsername: %s\n", mqttUsername);
    ESP_LOGW(TAG, "mqttPassword: %s\n", mqttPassword);
    ESP_LOGW(TAG, "mqttPasswordBase64: %s\n", cipherBase64); // 无用
    ESP_LOGW(TAG, "brokerUrl: %s\n", brokerUrl);
    ESP_LOGW(TAG, "topic_post: %s\n", topic_post);
    ESP_LOGW(TAG, "topic_set: %s\n", topic_set);
    #endif
}
