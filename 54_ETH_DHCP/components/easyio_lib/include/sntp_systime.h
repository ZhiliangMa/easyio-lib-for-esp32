#ifndef __SNTP_SYSTIME_H__
#define __SNTP_SYSTIME_H__

#include <sys/time.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_sntp.h"

void obtain_time(void);
void initialize_sntp(void);

#endif
