
#include "easyio.h"

void app_main(void)
{
    // led初始化，设置推挽输出，设置初始电平为0
    led_init(BLINK_GPIO, 0);

    while(1) {
        /*// Blink off (output low)
	    printf("Turning off the LED\n");
        led_off(BLINK_GPIO);
        vTaskDelay(200 / portTICK_PERIOD_MS);

        // Blink on (output high)
	    printf("Turning on the LED\n");
        led_on(BLINK_GPIO);
        vTaskDelay(200 / portTICK_PERIOD_MS);*/

        // LED状态闪烁 （更为简便的写法）
        led_blink(BLINK_GPIO);
        printf("LED state: %d\r\n", gpio_get_level(BLINK_GPIO));
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
