#ifndef __WIFI_SCAN_PRINT_H__
#define __WIFI_SCAN_PRINT_H__

#include <string.h>
#include "esp_wifi.h"
#include "esp_log.h"


void print_auth_mode(int authmode);
void print_cipher_type(int pairwise_cipher, int group_cipher);

#endif
