#include "http_get_weather.h"

/**
 * @brief  cjson解析心知天气报文
 * 
 * @param  rdata  心知天气http返回的JSON字符串指针
 * 
 * @return
 *     - none
 */
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

/**
 * @brief  cjson解析sojson天气报文
 * 
 * @param  rdata  sojson天气http返回的JSON字符串指针
 * 
 * @return
 *     - none
 */
void cjson_parse_sojson_weather(char *rdata)
{
    // 判断是否为json格式
    cJSON *pJsonRoot = cJSON_Parse(rdata);
    // 如果是json格式数据，则开始解析
    if (pJsonRoot != NULL)
    {
        // 解析 date，获得当前日期
        cJSON *pDate = cJSON_GetObjectItem(pJsonRoot, "date");
        if (pDate != NULL) {
            printf("%s - ", pDate->valuestring);
        }
        // 解析 cityInfo，获得省份、城市信息
        cJSON *pCityInfo = cJSON_GetObjectItem(pJsonRoot, "cityInfo");
		if(pCityInfo != NULL) {
            // 解析 cityInfo -> parent，获得 省份信息
            cJSON *pParent = cJSON_GetObjectItem(pCityInfo, "parent");
            if (pParent != NULL) {
                printf("%s - ", pParent->valuestring);
            }
            // 解析 cityInfo -> city，获得 城市信息
            cJSON *pCity = cJSON_GetObjectItem(pCityInfo, "city");
            if (pCity != NULL) {
                printf("%s\n", pCity->valuestring);
            }
        }

        // 解析 data，获得天气信息
        cJSON *pData = cJSON_GetObjectItem(pJsonRoot, "data");
        if (pData != NULL) {
            // 解析 data -> wendu，获得 当前温度
            cJSON *pWendu = cJSON_GetObjectItem(pData, "wendu");
            if (pWendu != NULL) {
                printf("当前: %s ℃ - ", pWendu->valuestring);
            }
            // 解析 data -> shidu，获得 当前湿度
            cJSON *pShidu = cJSON_GetObjectItem(pData, "shidu");
            if (pShidu != NULL) {
                printf("湿度: %s - ", pShidu->valuestring);
            }
            // pm25需要被解析为 valueint，如果解析为 valuestring，会在运行时报错重启
            // 解析 data -> pm25，获得 当前PM2.5
            cJSON *pPm25 = cJSON_GetObjectItem(pData, "pm25");
            if (pPm25 != NULL) {
                printf("PM2.5: %d - ", pPm25->valueint);
            }
            // 解析 data -> quality，获得 空气质量
            cJSON *pQuality = cJSON_GetObjectItem(pData, "quality");
            if (pQuality != NULL) {
                printf("AQI: %s\n", pQuality->valuestring);
            }

            // 解析 data -> forecast，获得 天气预报 数组
            cJSON *pForecast = cJSON_GetObjectItem(pData, "forecast");
            if (pForecast != NULL) {
                // 获取 forecast 的数组长度，理应为15
                int size = cJSON_GetArraySize(pForecast);
                printf("未来%d日天气:\n", size);

                // 解析 data -> forecast 各数组的内容，列表未来天气情况
                for (int i=0; i<size; i++) {
                    cJSON *pObject = cJSON_GetArrayItem(pForecast, i);
                    if (pObject != NULL) {
                        // 解析 data -> forecast[i] -> ymd，获得 预报天气的日期
                        cJSON *pYmd = cJSON_GetObjectItem(pObject, "ymd");
                        if (pYmd != NULL) {
                            printf("%s - ", pYmd->valuestring);
                        }
                        // 解析 data -> forecast[i] -> week，获得 预报天气的星期
                        cJSON *pWeek = cJSON_GetObjectItem(pObject, "week");
                        if (pWeek != NULL) {
                            printf("%s - ", pWeek->valuestring);
                        }
                        // 解析 data -> forecast[i] -> type，获得 预报天气的天气
                        cJSON *pType = cJSON_GetObjectItem(pObject, "type");
                        if (pType != NULL) {
                            printf("%s - ", pType->valuestring);
                        }
                        // 解析 data -> forecast[i] -> low，获得 预报天气的最低温度
                        cJSON *pLow = cJSON_GetObjectItem(pObject, "low");
                        if (pLow != NULL) {
                            printf("%s - ", pLow->valuestring);
                        }
                        // 解析 data -> forecast[i] -> high，获得 预报天气的最高温度
                        cJSON *pHigh = cJSON_GetObjectItem(pObject, "high");
                        if (pHigh != NULL) {
                            printf("%s - ", pHigh->valuestring);
                        }
                        // 解析 data -> forecast[i] -> fx，获得 预报天气的风向
                        cJSON *pFx = cJSON_GetObjectItem(pObject, "fx");
                        if (pFx != NULL) {
                            printf("%s - ", pFx->valuestring);
                        }
                        // 解析 data -> forecast[i] -> fl，获得 预报天气的风力
                        cJSON *pFl = cJSON_GetObjectItem(pObject, "fl");
                        if (pFl != NULL) {
                            printf("%s - ", pFl->valuestring);
                        }
                        // aqi需要被解析为 valueint，如果解析为 valuestring，会在运行时报错重启
                        // 解析 data -> forecast[i] -> aqi，获得 预报天气的空气质量
                        cJSON *pAqi = cJSON_GetObjectItem(pObject, "aqi");
                        if (pAqi != NULL) {
                            printf("AQI: %d\n", pAqi->valueint);
                        }
                    }
                }
            }
        }
    }
    cJSON_Delete(pJsonRoot); // 释放JSON对象
}
