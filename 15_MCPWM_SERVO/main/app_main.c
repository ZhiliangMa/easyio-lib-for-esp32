
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

// mcpwm_servo_task 任务，初始化6路舵机，servo0来回摆动，servo1~5的摆角各不相同
void mcpwm_servo_task(void* arg)
{
    //easyIO提供最多12个舵机的驱动。（注意：为保证多路舵机相位相等，可开启相位同步）
    //这里只使用 servo0~5
    //普通舵机的PWM频率 = 50Hz，可指定任意端口输出
    mcpwm_servo_init(servo_0_1_unit, 50, GPIO_SERVO0_OUT, GPIO_SERVO1_OUT);
    mcpwm_servo_init(servo_2_3_unit, 50, GPIO_SERVO2_OUT, GPIO_SERVO3_OUT);
    mcpwm_servo_init(servo_4_5_unit, 50, GPIO_SERVO4_OUT, GPIO_SERVO5_OUT);
    //为保证多路半桥驱动的PWM相位相等，开启同步。
    //同时选择MCPWM_SELECT_SYNC0作为同步信号源，使用GPIO_MC0_SYNC0_IN做同步信号源输入引脚
    mcpwm_servo_sync(servo_0_1_unit, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
    mcpwm_servo_sync(servo_2_3_unit, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
    mcpwm_servo_sync(servo_4_5_unit, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);

    //控制舵机的角度
    servo_degree(servo0, 0);
    servo_degree(servo1, 10);
    servo_degree(servo2, 25);
    servo_degree(servo3, 40);
    servo_degree(servo4, 55);
    servo_degree(servo5, 70);

    while (1) {
        //servo0来回摆动
        for(uint8_t i=0; i<(SERVO_MAX_DEGREE+1); i++) {
            servo_degree(servo0, i);
            vTaskDelay(20 / portTICK_RATE_MS);
        }
        for(uint8_t i=(SERVO_MAX_DEGREE+1); i>0; i--) {
            servo_degree(servo0, i);
            vTaskDelay(20 / portTICK_RATE_MS);
        }
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 mcpwm_servo_task 任务，运行任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(mcpwm_servo_task, "mcpwm_servo_task", 2048, NULL, 3, NULL);
}
