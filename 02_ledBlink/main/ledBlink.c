
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "led.h"

void app_main(void)
{
    led_init();
    while(1) {
        /* Blink off (output low) */
        printf("Turning off the LED\n");
        led_off();
        vTaskDelay(200 / portTICK_PERIOD_MS);

        /* Blink on (output high) */
        printf("Turning on the LED\n");
        led_on();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
