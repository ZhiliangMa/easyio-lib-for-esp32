
#include "easyio.h"

#define LED 33
#define KEY 0

// led_task 任务，控制LED闪烁
void led_task(void* arg)
{
    // 配置LED为推挽输出，设置初始电平为0
    led_init(LED, 0);
    while(1) {
        // LED状态闪烁
        led_blink(LED);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

// gpio_in_task 任务，串口输出按键电平值
void gpio_in_task(void* arg)
{
    // 配置KEY引脚为上拉输入
    gpiox_set_input(KEY, GPIO_PULLUP_ENABLE, GPIO_PULLDOWN_DISABLE);
    while(1) {
        // 串口输出按键KEY电平值
        printf("Key level?: %d\r\n", gpiox_get_level(KEY));
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, NULL);
    //创建 gpio_in_task 任务，任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(gpio_in_task, "gpio_in_task", 2048, NULL, 3, NULL);
}
