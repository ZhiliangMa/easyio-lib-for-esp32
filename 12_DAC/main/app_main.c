
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

// dac_output_task 任务，初始化DAC_CHANNEL_1(GPIO25)；并生成三角波，并用ADC1-CHx测量输出电压
void dac_output_task(void* arg)
{
    uint32_t output_voltage = 0;

    //DAC输出通道1初始化
    dac_channel_init(DAC_CHANNEL_1);
    //ADC1及输入通道初始化
    adc1_init_with_calibrate(ADC_ATTEN_DB_11, 1, ADC_CHANNEL_6);

    while (1) {
        //DAC通道x输出电压xxmV
        dac_output_0_3V3_voltage_int_mV(DAC_CHANNEL_1, output_voltage);
        //读取ADC1通道x的 经16次多重采样并校准补偿后的电压值
        uint32_t mul = adc1_cal_get_voltage_mul(ADC_CHANNEL_6, 16);
        printf("DAC_output: %dmV\tADC: %dmV\n", output_voltage, mul);

        //自增，生成三角波
        output_voltage += 10;
        if(output_voltage > 3300) output_voltage = 0;

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 dac_output_task 任务，运行任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(dac_output_task, "dac_output_task", 2048, NULL, 3, NULL);
}
