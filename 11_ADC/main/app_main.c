
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

// adc1_scan_task 任务，扫描ADC1的两个2个通道：电位器、光敏电阻。并控制led任务运行状态
void adc1_scan_task(void* arg)
{
    //ADC1及输入通道初始化
    //不同的衰减系数，会影响经校准补偿后的值。且衰减越大，对读数准确性的影响也越大。
    //例如我其中一个Demo板为：0：75~1008mV。2.5：78~1317mV。6：107~1821mV。11：142~3108mV。
    //可以通过设置更高的衰减来扩展输入范围，但推荐使用更低的衰减以提高校准后值的准确性。
    //adc1_init_with_calibrate(ADC_ATTEN_DB_0, 1, ADC_CHANNEL_6);
    //adc1_init_with_calibrate(ADC_ATTEN_DB_2_5, 1, ADC_CHANNEL_6);
    //adc1_init_with_calibrate(ADC_ATTEN_DB_6, 1, ADC_CHANNEL_6);
    //adc1_init_with_calibrate(ADC_ATTEN_DB_11, 2, ADC_CHANNEL_6, ADC_CHANNEL_7);
    adc1_init_with_calibrate(ADC_ATTEN_DB_11, 1, ADC_CHANNEL_3);

    while (1) {
        //读取ADC1通道x的 原始值、经校准补偿后的电压值、经64次多重采样并校准补偿后的电压值
        uint32_t raw = adc1_get_raw(ADC_CHANNEL_3);
        uint32_t cal = adc1_cal_get_voltage(ADC_CHANNEL_3);
        uint32_t mul = adc1_cal_get_voltage_mul(ADC_CHANNEL_3, 64);
        printf("Raw: %d\tVoltage: %dmV\tMul: %dmV\t\n", raw, cal, mul);
        //uint32_t mul2 = adc1_cal_get_voltage_mul(ADC_CHANNEL_3, 64);
        //printf("Raw: %d\tVoltage: %dmV\tMul: %dmV\tMul2: %dmV\n", raw, cal, mul, mul2);

        //用物体遮挡光敏电阻或处于黑暗环境下，adc读数表小，LED继续闪烁。
        if(mul < 500) {
            if(eTaskGetState(led_task_Handler) != eBlocked) {
                vTaskResume(led_task_Handler);//继续任务
            }
        }else{ //光敏电阻受光后，adc读数增大，LED停止闪烁。
            if(eTaskGetState(led_task_Handler) != eSuspended) {
                vTaskSuspend(led_task_Handler);//暂停任务
            }
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    //创建 adc1_scan_task 任务，运行任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(adc1_scan_task, "adc1_scan_task", 2048, NULL, 3, NULL);
}
