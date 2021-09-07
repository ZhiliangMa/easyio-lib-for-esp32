
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

// mcpwm_half_bridge_task 任务，初始化3路半桥驱动，PWMA/PWMB反相互补输出，观察示波器的通道波形。连接电机会有啸叫。
void mcpwm_half_bridge_task(void* arg)
{
    //easyIO提供最多6个半桥驱动的驱动，共12路PWM。（注意：为保证多路半桥相位相等，可开启相位同步）
    //这里只使用 half_bridge0~2
    //PWM频率 = 1000Hz，PWMA/PWMB反相，可指定任意端口输出
    mcpwm_half_bridge_init(half_bridge0, 1000, inverted, GPIO_HALF_BRIDGE0_PWMA_OUT, GPIO_HALF_BRIDGE0_PWMB_OUT);
    mcpwm_half_bridge_init(half_bridge1, 1000, inverted, GPIO_HALF_BRIDGE1_PWMA_OUT, GPIO_HALF_BRIDGE1_PWMB_OUT);
    mcpwm_half_bridge_init(half_bridge2, 1000, inverted, GPIO_HALF_BRIDGE2_PWMA_OUT, GPIO_HALF_BRIDGE2_PWMB_OUT);
    //为保证多路半桥驱动的PWM相位相等，开启同步。半桥2的两个PWM演示20%相位左移
    //三路半桥同时选择MCPWM_SELECT_SYNC0作为同步信号源，使用GPIO_MC0_SYNC0_IN做同步信号源输入引脚。half_bridge2相位左移20％
    mcpwm_half_bridge_sync(half_bridge0, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
    mcpwm_half_bridge_sync(half_bridge1, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
    mcpwm_half_bridge_sync(half_bridge2, MCPWM_SELECT_SYNC0, 200, GPIO_MC0_SYNC0_IN);

    //调整half_bridge1、2的PWM占空比，这里的四个GPIO只做PWM输出和相位调整演示
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 20.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, 40.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, 60.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B, 80.0);

    while (1) {
        //半桥例程，死区没调通，一用死区就会破坏反相，只能单独使用，暂记作bug
        //half_bridge0输出反相且连续变化的PWM，电机如有连接会有啸叫
        for(uint16_t i=0; i<100; i++) {
            mcpwm_half_bridge_output(half_bridge0, i, i);
            vTaskDelay(50 / portTICK_RATE_MS);
        }
        for(uint16_t i=0; i<50; i++) {
            mcpwm_half_bridge_output(half_bridge0, i, 100-i);
            vTaskDelay(50 / portTICK_RATE_MS);
        }
        //half_bridge2相位连续发生变化
        for(uint16_t i=0; i<100; i++) {
            mcpwm_half_bridge_change_phase(half_bridge2, MCPWM_SELECT_SYNC0, i*10);
            vTaskDelay(50 / portTICK_RATE_MS);
        }
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 mcpwm_half_bridge_task 任务，运行任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(mcpwm_half_bridge_task, "mcpwm_half_bridge_task", 2048, NULL, 3, NULL);
}
