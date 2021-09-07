
#include "easyio.h"

#define LED 33
#define KEY 0 // BOOT按键为GPIO0，正常启动后可以作用户按键。USR按键为 GPIO36 - SENSOR_VP，不能设置为上拉，需要外部电阻上拉

void app_main(void)
{
    uint8_t light = 0;
    // 配置LED为推挽输出，设置初始电平为0
    gpiox_set_ppOutput(LED, 0);
    // 配置KEY引脚为上拉输入
    gpiox_set_input(KEY, GPIO_PULLUP_ENABLE, GPIO_PULLDOWN_DISABLE);
    while(1) {
        light = !light;
        // 设置LED引脚输出电平
        gpiox_set_level(LED, light);
        // 串口输出LED亮灭，和按键KEY电平值
        printf("LED state: %d, Key level?: %d\r\n", light, gpiox_get_level(KEY));
        
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
