#include "http_get_weather.h"


// cjson解析心知天气报文
void cjson_parse_xinzhi_weather(char *rdata)
{
    // 判断是否为json格式
    cJSON *pJsonRoot = cJSON_Parse(rdata);
    // 如果是json格式数据，则开始解析
    if (pJsonRoot != NULL)
    {
        // 解析 results，获得 天气预报 数组
        cJSON *pResults = cJSON_GetObjectItem(pJsonRoot, "results");
		if(pResults != NULL)
		{
            // 获取 results 的数组长度，理应为1
			int size = cJSON_GetArraySize(pResults);
			printf("cJSON_GetArraySize: size=%d\n", size);

            // 解析 results 数组的内容
            cJSON *pObject = cJSON_GetArrayItem(pResults, 0);
            if (pObject != NULL) {
                // 解析 results -> location，获得 地理信息
                cJSON *pLocation = cJSON_GetObjectItem(pObject, "location");
                if (pLocation != NULL) {
                    // 解析 results -> location ->path，获得 城市信息
                    cJSON *pPath = cJSON_GetObjectItem(pLocation, "path");
                    if (pPath != NULL) {
                        printf("\n %s \n", pPath->valuestring);
                    }
                }

                // 解析 results -> daily 各数组的内容，列表未来天气情况
                cJSON *pDaily = cJSON_GetObjectItem(pObject, "daily");
                if (pDaily != NULL) {
                    // 获取 daily 的数组长度，免费用户理应为3，为今、明、后的3天消息
                    size = cJSON_GetArraySize(pDaily);
                    printf("未来%d日天气:\n", size);

                    // 解析 results -> daily 各数组的内容，列表未来天气情况
                    for (int i=0; i<size; i++) {
                        cJSON *pObject = cJSON_GetArrayItem(pDaily, i);
                        if (pObject != NULL) {
                            // 解析 results -> daily[i] -> date，获得 预报天气的日期
                            cJSON *pDate = cJSON_GetObjectItem(pObject, "date");
                            if (pDate != NULL) {
                                printf("%s\t", pDate->valuestring);
                            }
                            // 解析 results -> daily[i] -> low，获得 最低温度
                            cJSON *pLow = cJSON_GetObjectItem(pObject, "low");
                            if (pLow != NULL) {
                                printf("%s - ", pLow->valuestring);
                            }
                            // 解析 results -> daily[i] -> high，获得 最高温度
                            cJSON *pHigh = cJSON_GetObjectItem(pObject, "high");
                            if (pHigh != NULL) {
                                printf("%s ℃\t", pHigh->valuestring);
                            }
                            // 解析 results -> daily[i] -> text_day，获得 白天天气
                            cJSON *pDay = cJSON_GetObjectItem(pObject, "text_day");
                            if (pDay != NULL) {
                                printf("%s  ", pDay->valuestring);
                            }
                            // 解析 results -> daily[i] -> text_night，获得 晚间天气
                            cJSON *pNight = cJSON_GetObjectItem(pObject, "text_night");
                            if (pNight != NULL) {
                                printf("%s\n", pNight->valuestring);
                            }
                        }
                    }
                }
            }
        }
    }
    cJSON_Delete(pJsonRoot); // 释放JSON对象
}
