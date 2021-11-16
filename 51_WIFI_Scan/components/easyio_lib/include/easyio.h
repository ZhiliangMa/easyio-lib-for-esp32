#ifndef __EASYIO_H__
#define __EASYIO_H__

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include <dirent.h>

#include "led.h"
#include "gpioX.h"
#include "key.h"
#include "touch_pad_button.h"
#include "ledc_pwm.h"
#include "adc_sampling.h"
#include "dac_output.h"
#include "mcpwm_motor.h"
#include "mcpwm_half_bridge.h"
#include "mcpwm_servo.h"
#include "mcpwm_capture.h"
#include "pulse_cnt.h"
#include "rmt_ir.h"
#include "esp_log.h"
#include "ir_tools.h"
#include "rmt_ws2812b.h"
#include "uart_config.h"
#include "cmd_i2ctools.h"
#include "i2c_config.h"
#include "i2c_mpu6050.h"
#include "i2c_aht20.h"
#include "i2c_sht30.h"
#include "i2c_pcf8563.h"
#include "spi_config.h"
#include "spi_lcd.h"
#include "simple_gui.h"
#include "picture.h"
#include "spi_as5047p.h"
#include "spi_tle5012b.h"
#include "i2c_ctp_ftxx06_xx36.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "sd_card_fatfs.h"
#include "jpg_decode.h"
#include "lcd_spi_dma.h"
#include "http_get_weather.h"
#include "mbedtls_encrypt.h"
#include "mqtt_passwd_generate.h"
#include "aliyun_iot_event.h"
#include "sntp_systime.h"
#include "wifi_scan_print.h"

#include <string.h>
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include <sys/param.h>
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "addr_from_stdin.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "cJSON.h"
#include <stdint.h>
#include <stddef.h>
#include "mqtt_client.h"


#endif
