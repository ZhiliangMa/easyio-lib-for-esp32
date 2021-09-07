
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

// mcpwm_dc_motor_task 任务，初始化3路电机PWM信号，其中一路驱动硬件H桥芯片DRVxxxx，使电机正转反转。
// PWM输出引脚、同步信号输入引脚 请查看 easyio_mcpwm_config.h 的定义
void mcpwm_dc_motor_task(void* arg)
{
    //easyIO提供最多6个DC直流有刷电机的驱动，共12路PWM。（注意：为保证多路有刷电机的PWM相位相等，可开启相位同步）
    //这里只使用 motor0~2
    //PWM频率 = 1000Hz，可指定任意端口输出
    mcpwm_dc_motor_init(motor0, 1000, GPIO_MOTOR0_PWMA_OUT, GPIO_MOTOR0_PWMB_OUT);
    mcpwm_dc_motor_init(motor1, 1000, GPIO_MOTOR1_PWMA_OUT, GPIO_MOTOR1_PWMB_OUT);
    mcpwm_dc_motor_init(motor2, 1000, GPIO_MOTOR2_PWMA_OUT, GPIO_MOTOR2_PWMB_OUT);
    //为保证多路有刷电机的PWM相位相等，开启同步，并将同步信号源GPIO与一路PWM输出连接。motor2的两个PWM演示20%相位左移
    //三路电机同时选择MCPWM_SELECT_SYNC0作为同步信号源，使用GPIO_MC0_SYNC0_IN做同步信号源输入引脚。motor2相位左移20％
    mcpwm_dc_motor_sync(motor0, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
    mcpwm_dc_motor_sync(motor1, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
    mcpwm_dc_motor_sync(motor2, MCPWM_SELECT_SYNC0, 200, GPIO_MC0_SYNC0_IN);

    //调整motor1、2的PWM占空比，这里的四个GPIO不做驱动电机，只做PWM输出及相位演示
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 20.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, 40.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, 60.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B, 80.0);

    while (1) {
        //motor0正转、反转、停止
        dc_motor_forward(motor0, 50.0);
        vTaskDelay(2000 / portTICK_RATE_MS);
        dc_motor_backward(motor0, 30.0);
        vTaskDelay(2000 / portTICK_RATE_MS);
        dc_motor_stop(motor0);
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 mcpwm_dc_motor_task 任务，运行任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(mcpwm_dc_motor_task, "mcpwm_dc_motor_task", 2048, NULL, 3, NULL);
}
