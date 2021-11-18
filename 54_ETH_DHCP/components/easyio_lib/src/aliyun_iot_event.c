#include "aliyun_iot_event.h"

static const char *TAG = "aliyun";

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
char *aliyun_iot_Temperature_Humidity_json_splice(double temperature, int humidity)
{
    cJSON *pRoot = cJSON_CreateObject(); // 创建JSON根结构体
    cJSON *pSon = cJSON_CreateObject(); // 创建JSON子结构体

    cJSON_AddNumberToObject(pRoot, "id", random()); // 添加整型数据到根结构体。消息id，使用随机数

    cJSON_AddItemToObject(pRoot, "params", pSon); // 添加子结构体到根
    cJSON_AddNumberToObject(pSon, "CurrentTemperature", temperature); // 添加数据到子结构体，阿里云物联网平台温度为double
    cJSON_AddNumberToObject(pSon, "CurrentHumidity", humidity); // 添加数据到子结构体，阿里云物联网平台湿度为整形

    cJSON_AddStringToObject(pRoot, "version", "1.0"); // 添加字符串类型数据到根结构体
    cJSON_AddStringToObject(pRoot, "method", "thing.event.property.post"); // 添加字符串类型数据到根结构体

    //char *sendData = cJSON_Print(pRoot); // 格式化输出的JSON对象
    char *payload = cJSON_PrintUnformatted(pRoot); // 未格式化的，原文
    printf("data:\n%s\n", payload);
    //cJSON_free((void *) payload); // 释放cJSON_Print ()分配的内存
    cJSON_Delete(pRoot); // 释放cJSON_CreateObject ()分配的内存
    return payload; // 返回json字符串指针。【注意】：使用此函数过后，请务必释放返回指针的内存，以避免重复调用造成内存溢出
}

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
char *aliyun_iot_Temperature_Humidity_mlux_Voltage_json_splice(double temperature, int humidity, int mlux, double Voltage)
{
    cJSON *pRoot = cJSON_CreateObject(); // 创建JSON根结构体
    cJSON *pSon = cJSON_CreateObject(); // 创建JSON子结构体

    cJSON_AddNumberToObject(pRoot, "id", random()); // 添加整型数据到根结构体。消息id，使用随机数

    cJSON_AddItemToObject(pRoot, "params", pSon); // 添加子结构体到根
    cJSON_AddNumberToObject(pSon, "CurrentTemperature", temperature); // 添加数据到子结构体，阿里云物联网平台温度为double
    cJSON_AddNumberToObject(pSon, "CurrentHumidity", humidity); // 添加数据到子结构体，阿里云物联网平台湿度为整形
    cJSON_AddNumberToObject(pSon, "mlux", mlux); // 添加数据到子结构体，阿里云物联网平台光照度为整形
    cJSON_AddNumberToObject(pSon, "CurrentVoltage", Voltage); // 添加数据到子结构体，阿里云物联网平台湿度为整形

    cJSON_AddStringToObject(pRoot, "version", "1.0"); // 添加字符串类型数据到根结构体
    cJSON_AddStringToObject(pRoot, "method", "thing.event.property.post"); // 添加字符串类型数据到根结构体

    //char *sendData = cJSON_Print(pRoot); // 格式化输出的JSON对象
    char *payload = cJSON_PrintUnformatted(pRoot); // 未格式化的，原文
    printf("data:\n%s\n", payload);
    //cJSON_free((void *) payload); // 释放cJSON_Print ()分配的内存
    cJSON_Delete(pRoot); // 释放cJSON_CreateObject ()分配的内存
    return payload; // 返回json字符串指针。【注意】：使用此函数过后，请务必释放返回指针的内存，以避免重复调用造成内存溢出
}

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
char *aliyun_iot_Temperature_Humidity_mlux_Voltage_statusLight_json_splice(double temperature, int humidity, int mlux, double Voltage, int statusLight)
{
    cJSON *pRoot = cJSON_CreateObject(); // 创建JSON根结构体
    cJSON *pSon = cJSON_CreateObject(); // 创建JSON子结构体

    cJSON_AddNumberToObject(pRoot, "id", random()); // 添加整型数据到根结构体。消息id，使用随机数

    cJSON_AddItemToObject(pRoot, "params", pSon); // 添加子结构体到根
    cJSON_AddNumberToObject(pSon, "CurrentTemperature", temperature); // 添加数据到子结构体，阿里云物联网平台温度为double
    cJSON_AddNumberToObject(pSon, "CurrentHumidity", humidity); // 添加数据到子结构体，阿里云物联网平台湿度为整形
    cJSON_AddNumberToObject(pSon, "mlux", mlux); // 添加数据到子结构体，阿里云物联网平台光照度为整形
    cJSON_AddNumberToObject(pSon, "CurrentVoltage", Voltage); // 添加数据到子结构体，阿里云物联网平台电压为double
    cJSON_AddNumberToObject(pSon, "StatusLightSwitch", statusLight); // 添加数据到子结构体，阿里云物联网平台状态灯运行状态为bool

    cJSON_AddStringToObject(pRoot, "version", "1.0"); // 添加字符串类型数据到根结构体
    cJSON_AddStringToObject(pRoot, "method", "thing.event.property.post"); // 添加字符串类型数据到根结构体

    //char *sendData = cJSON_Print(pRoot); // 格式化输出的JSON对象
    char *payload = cJSON_PrintUnformatted(pRoot); // 未格式化的，原文
    printf("data:\n%s\n", payload);
    //cJSON_free((void *) payload); // 释放cJSON_Print ()分配的内存
    cJSON_Delete(pRoot); // 释放cJSON_CreateObject ()分配的内存
    return payload; // 返回json字符串指针。【注意】：使用此函数过后，请务必释放返回指针的内存，以避免重复调用造成内存溢出
}

/**
 * @brief  cjson解析阿里云物联网平台的 `状态灯开关` 控制消息
 * 
 * @param  rdata  阿里云物联网平台MQTT收到的控制json报文
 * 
 * @return
 *     - int。状态灯的状态。0/1。错误时返回-1。
 */
int cjson_parse_aliyun_iot_StatusLightSwitch(char *rdata)
{
    // 判断是否为json格式
    cJSON *pJsonRoot = cJSON_Parse(rdata);
    // 如果是json格式数据，则开始解析
    if (pJsonRoot != NULL) {
        // 解析 params，获得 物模型参数
        cJSON *pParams = cJSON_GetObjectItem(pJsonRoot, "params");
		if(pParams != NULL) {
            // 解析 params -> StatusLightSwitch，获得 状态灯开关 状态
            cJSON *pStatusLightSwitch = cJSON_GetObjectItem(pParams, "StatusLightSwitch");
            if (pStatusLightSwitch != NULL) {
                int statusLight = pStatusLightSwitch->valueint;
                ESP_LOGW(TAG, "\nStatusLightSwitch: %d \n", statusLight);
                cJSON_Delete(pJsonRoot); // 释放cJSON_Parse ()分配的内存
                return statusLight;
            }
        }
    }
    cJSON_Delete(pJsonRoot); // 释放cJSON_Parse ()分配的内存
    return -1;
}
