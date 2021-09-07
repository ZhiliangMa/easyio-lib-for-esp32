
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

// mcpwm_capture_task 任务。初始化6路1KHz-PWM，作为测试信号，使能6路输入捕获，来测试输出与捕获的准确性
void mcpwm_capture_task(void* arg)
{
    //easyIO提供最多6个DC直流有刷电机的驱动，共12路PWM。（注意：为保证多路有刷电机的PWM相位相等，可开启相位同步）
    //这里只使用 motor0~2
    //PWM频率 = 1000Hz，可指定任意端口输出
    mcpwm_dc_motor_init(motor0, 1000, GPIO_MOTOR0_PWMA_OUT, GPIO_MOTOR0_PWMB_OUT);
    mcpwm_dc_motor_init(motor1, 1000, GPIO_MOTOR1_PWMA_OUT, GPIO_MOTOR1_PWMB_OUT);
    mcpwm_dc_motor_init(motor2, 1000, GPIO_MOTOR2_PWMA_OUT, GPIO_MOTOR2_PWMB_OUT);
    //为保证多路有刷电机的PWM相位相等，开启同步。motor2的两个PWM演示20%相位左移
    //三路电机同时选择MCPWM_SELECT_SYNC0作为同步信号源，使用GPIO_MC0_SYNC0_IN做同步信号源输入引脚。motor2相位左移20％
    mcpwm_dc_motor_sync(motor0, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
    mcpwm_dc_motor_sync(motor1, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
    mcpwm_dc_motor_sync(motor2, MCPWM_SELECT_SYNC0, 200, GPIO_MC0_SYNC0_IN);

    //调整motor0、1、2的PWM占空比，这里的6路GPIO不用做驱动电机，只做PWM输出演示
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 5.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 10.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 20.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, 40.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, 60.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B, 80.0);

    //开启输入捕获，计算6个输入通道的周期、频率、占空比
    //mcpwm_capture_duty_cycle_init(3, GPIO_CAPTURE0_IN, GPIO_CAPTURE1_IN, GPIO_CAPTURE2_IN);
    mcpwm_capture_duty_cycle_init(6, GPIO_CAPTURE0_IN, GPIO_CAPTURE1_IN, GPIO_CAPTURE2_IN, GPIO_CAPTURE3_IN, GPIO_CAPTURE4_IN, GPIO_CAPTURE5_IN);

    while (1) {
        //调试输出各输入捕获通道的 周期、频率、单个脉宽内高电平的时间、占空比
        for (int i=0;i<6;i++) {
            printf("CAP%d Period: %lld us\tFreq: %.2f Hz\tHigh_level: %lld us\tDuty_cycle: %.2f%%\n", i,
                                                                capture.period[i]/1000,
                                                                capture.freq[i],
                                                                capture.high_level_val[i]/1000,
                                                                capture.duty_cycle[i]);
        }

        printf("\n");
        vTaskDelay(200 / portTICK_RATE_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 mcpwm_capture_task 任务，运行任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(mcpwm_capture_task, "mcpwm_capture_task", 2048, NULL, 3, NULL);
}
