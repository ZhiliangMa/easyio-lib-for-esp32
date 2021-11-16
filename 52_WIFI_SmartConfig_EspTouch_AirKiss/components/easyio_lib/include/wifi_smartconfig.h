#ifndef __WIFI_SMARTCONFIG_H__
#define __WIFI_SMARTCONFIG_H__

#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "nvs_flash.h"
#include "nvs.h"

#define RETYR_SSID_NUM    5  // 检测SSID密码错误的重试次数


// 初始化WIFI。配置 SmartConfig 事件回调、启动WIFI、从NVS读取SSID密码，密码错误后自动切换SmartConfig。无需额外的按键辅助配网。
void initialise_wifi_smartconfig(void);

#endif
