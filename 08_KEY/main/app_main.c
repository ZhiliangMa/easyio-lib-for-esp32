
#include "easyio.h"

#define LED 33
#define KEY 0

// 任务句柄，包含创建任务的所有状态，对任务的操作都通过操作任务句柄实现
TaskHandle_t led_task_Handler;

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

// key_scan_task 任务，单独运行一个任务，运行按键检测（滤除按键抖动、短按、长按）
void key_scan_task(void* arg)
{
    //按键检测任务
    key_scan(1, KEY);
}

// key_catch_task 任务。去捕获按键事件，并控制LED任务状态。
void key_catch_task(void* arg)
{
    uint32_t key_event = 0;
    uint32_t key_gpio = 0; // 触发按键事件的按键gpio序号
    uint32_t key_type = 0; // 触发按键事件的类型

    while(1) {
        // 以阻塞方式，不断读取 key_evt_queue 队列信息，监控按键事件
        if (xQueueReceive(key_evt_queue, &key_event, portMAX_DELAY)) {
            // 拆分按键事件，得到按键值和按键类型
            key_gpio = key_event & 0x0000FFFF; // 按键值
            key_type = key_event >> 16; // 按键类型（1为短按，2为长按）

            if(key_gpio == KEY) {
                if(key_type == 1) { // 1，短按，暂停LED任务
                    vTaskSuspend(led_task_Handler); // 暂停任务，LED停止闪烁
                }else if(key_type == 2) { // 2，长按，继续LED任务
                    vTaskResume(led_task_Handler); // 继续任务，LED继续闪烁
                }
                printf("key:%d, type:%d\n", key_gpio, key_type); // 输出按键事件
            }
        }
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 key_scan_task 任务，运行任务栈空间大小为 4096，任务优先级为3。实测分配小于2048会导致开机反复重启
    xTaskCreate(key_scan_task, "key_scan_task", 4096, NULL, 3, NULL);
    // 创建 key_catch_task 任务，任务栈空间大小为 2048，任务优先级为3。去捕获按键事件，并控制LED任务状态。
    xTaskCreate(key_catch_task, "key_catch_task", 2048, NULL, 2, NULL);
}
