
#include "led.h"

void led_init(void)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    led_off();
}

void led_on(void)
{
    gpio_set_level(BLINK_GPIO, 1);
}

void led_off(void)
{
    gpio_set_level(BLINK_GPIO, 0);
}

void led_blink(void)
{
    static uint8_t level = 0;
    level = !level;
    gpio_set_level(BLINK_GPIO, level);
}
