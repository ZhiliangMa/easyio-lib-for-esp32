
#include "easyio.h"

#define LED 33
#define KEY 0

// 任务句柄，包含创建任务的所有状态，对任务的操作都通过操作任务句柄实现
TaskHandle_t led_task_Handler = NULL;

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

//touch_pad_scan_task 任务，检测触摸按键，并控制led任务运行状态
void touch_pad_scan_task(void* arg)
{
    // 初始化一个IO为touch pad，触摸传感器信号名为T5，对应管脚GPIO12
    // 此函数可多参数重载，以支持多触摸信号输入
    touch_pad_button_init(1, TOUCH_PAD_NUM5);
    while(1)
    {
        uint16_t value = 0;
        uint8_t touch_state[10] = {0};
        //获取全部10个通道的 的初始感应值、当前计数、触摸状态
        for(int i=0;i<TOUCH_PAD_MAX;i++) {
            //读取滤波后的OUT计数值
            touch_pad_read_filtered(i, &value);
            //记录当前的触摸状态
            touch_state[i] = s_pad_activated[i];
            printf("[TP%d, Init=%d, Val=%d B=%d] ", i, s_pad_init_val[i], value, touch_state[i]);
            // 清除触摸传感器的活动状态
            s_pad_activated[i] = false;
        }
        printf("\n\n");

        if(touch_state[TOUCH_PAD_NUM5]) { //1，有人体触摸
            if(eTaskGetState(led_task_Handler) != eSuspended) {
                vTaskSuspend(led_task_Handler);//暂停任务
            }
        }else { //0，电极空闲
            if(eTaskGetState(led_task_Handler) != eBlocked) {
                vTaskResume(led_task_Handler);//继续任务
            }
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    //创建 touch_pad_scan_task 任务，运行任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(touch_pad_scan_task, "touch_pad_scan_task", 2048, NULL, 3, NULL);
}
