#pragma once

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"


extern EventGroupHandle_t eth_event_group;
extern int GOTIP_BIT;


/**
 * @brief  注册以太网驱动。（PHY型号、引脚、时钟，请通过menuconfig配置）
 *         以太网注册成功，并获得到IP后，会将 eth_event_group 事件组的 GOTIP_BIT 置位。用户可由此事件组进行后续的事件处理
 * 
 * @param  void
 * 
 * @return
 *     - none
 */
void register_ethernet(void);
