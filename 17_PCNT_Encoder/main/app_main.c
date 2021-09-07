
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

// pcnt_encoder_count_task 任务。初始化PCNT_UNIT_0，进行ABZ正交编码器、Step/Direction步进/方向 编码器的计数
// 并通过时间系数、距离系数，获取累计计数、速度、距离
void pcnt_encoder_count_task(void* arg)
{
    //启动 pulse_cnt 计数器，ABZ正交解码模式，1倍频计数
    //pcnt_encoder_ABZ_1_period_1_cnt_init(PCNT_UNIT_0, 100, ENCODER0_INPUT_A, ENCODER0_INPUT_B);
    //启动 pulse_cnt 计数器，ABZ正交解码模式，4倍频，计数所有跳变沿
    pcnt_encoder_ABZ_1_period_4_cnt_init(PCNT_UNIT_0, 100, ENCODER0_INPUT_A, ENCODER0_INPUT_B);
    //启动 pulse_cnt 计数器，Step/Direction步进/方向模式，2倍频，计数所有跳变沿
    //pcnt_encoder_Step_Direction_1_period_2_cnt_init(PCNT_UNIT_0, 100, ENCODER0_INPUT_A, ENCODER0_INPUT_B);

    while (1) {
        //获得PCNT计数值，仅测试用
        int16_t count = 0;
        pcnt_get_counter_value(PCNT_UNIT_0, &count);
        //直接用esayIO的API，获取累计计数、速度、距离
        cal_pcnt_count_speed_dis(PCNT_UNIT_0, 1, 200);

        printf("PCNT cnt :%d\tTotal cnt :%lld    \tSpeed :%.2f\tDis :%.2f\n", count, pcnt_count[0].count, pcnt_count[0].speed, pcnt_count[0].dis);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

// encoder_simulation_task 任务，用mcpwm来模拟编码器的+-90°正交编码输出，100Hz
void encoder_simulation_task(void* arg)
{
    //使能两路半桥输出，相位交错90°，来模拟正交编码器信号
    //GPIO_HALF_BRIDGE0_PWMA_OUT 与 GPIO_HALF_BRIDGE1_PWMA_OUT 为一对正交编码器输出。或者 GPIO_HALF_BRIDGE0_PWMB_OUT 和 GPIO_HALF_BRIDGE1_PWMB_OUT
    //PWM频率 = 100Hz，同相输出，指定输出端口xxxx（实测MCPWM的频率设置为低于20Hz时会不准，如需更低建议使用LEDC_PWM）
    mcpwm_half_bridge_init(half_bridge0, 100, inphase, GPIO_HALF_BRIDGE0_PWMA_OUT, GPIO_HALF_BRIDGE0_PWMB_OUT);
    mcpwm_half_bridge_init(half_bridge1, 100, inphase, GPIO_HALF_BRIDGE1_PWMA_OUT, GPIO_HALF_BRIDGE1_PWMB_OUT);
    //使能同步，B相位左移270°，等同于右移90°
    mcpwm_half_bridge_sync(half_bridge0, MCPWM_SELECT_SYNC0, 0, GPIO_MC0_SYNC0_IN);
    mcpwm_half_bridge_sync(half_bridge1, MCPWM_SELECT_SYNC0, 750, GPIO_MC0_SYNC0_IN);

    //模仿正交编码器的输出信号，50%占空比
    mcpwm_half_bridge_output(half_bridge0, 50.0, 50.0);
    mcpwm_half_bridge_output(half_bridge1, 50.0, 50.0);
    vTaskDelay(5000 / portTICK_RATE_MS);
    /*//相位左移90°
    mcpwm_half_bridge_change_phase(half_bridge1, MCPWM_SELECT_SYNC0, 250);
    //相位左移270°，等同于右移90°
    mcpwm_half_bridge_change_phase(half_bridge1, MCPWM_SELECT_SYNC0, 750);*/

    while (1) {
        printf("\nPhase -90°\n");
        //相位左移90°
        mcpwm_half_bridge_change_phase(half_bridge1, MCPWM_SELECT_SYNC0, 250);
        vTaskDelay(10000 / portTICK_RATE_MS);

        printf("\nPhase +90°\n");
        //相位左移270°，等同于右移90°
        mcpwm_half_bridge_change_phase(half_bridge1, MCPWM_SELECT_SYNC0, 750);
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 pcnt_encoder_count_task 任务，运行任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(pcnt_encoder_count_task, "pcnt_encoder_count_task", 2048, NULL, 3, NULL);
    // 创建 encoder_simulation_task 任务，任务栈空间大小为 configMINIMAL_STACK_SIZE，任务优先级为3
    xTaskCreate(encoder_simulation_task, "encoder_simulation_task", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
}
