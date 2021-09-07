
#include "easyio.h"

//ledc_pwm_hs_rgb3ch_task 任务。使用高速ledc通道，控制RGB-LED呼吸渐变
void ledc_pwm_hs_rgb3ch_task(void* arg)
{
    int ch=0;
    //初始化高速ledc通道，5KHz，13Bit分辨率，共使用3个通道，对应RGB-LED的管脚
    ledc_pwm_hs_init();
    while (1) {
        //3s内：高速LEDc，0~2通道，0->50%占空比渐变（满分辨率为8191）
        printf("(1) LEDC HS_RGB fade up to duty = %d%%\n", 50);
        for (ch = 0; ch < LEDC_HS_CH_NUM; ch++) {
            ledc_pwm_set_fade_duty_cycle(ledc_hs_ch, ch, 500, LEDC_TEST_FADE_TIME);
        }
        vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);
        
        //3s内：高速LEDc，0~2通道，50%->0渐变
        printf("(2) LEDC HS_RGB fade down to duty = 0%%\n");
        for (ch = 0; ch < LEDC_HS_CH_NUM; ch++) {
            ledc_pwm_set_fade_duty_cycle(ledc_hs_ch, ch, 0, LEDC_TEST_FADE_TIME);
        }
        vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);
    }
}

//ledc_pwm_ls_led1ch_task 任务。使用低速ledc通道，控制单色LED以固定占空比输出
void ledc_pwm_ls_led1ch_task(void* arg)
{
    int ch=0;
    //初始化低速ledc通道，5KHz，13Bit分辨率，共使用1个通道，对应单色LED的管脚
    ledc_pwm_ls_init();
    while (1) {
        //低速LEDc，1通道，设置为固定占空比输出：0（满分辨率为8191）
        printf("\t\t\t\t\t[1] LEDC LS_LED set duty = %d\%% without fade\n", 0);
        for (ch = 0; ch < LEDC_LS_CH_NUM; ch++) {
            ledc_pwm_set_duty_cycle(ledc_ls_ch, ch, 0);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        //低速LEDc，1通道，设置为固定占空比输出：50%（满分辨率为8191）
        printf("\t\t\t\t\t[2] LEDC LS_LED set duty = %d\%% without fade\n", 50);
        for (ch = 0; ch < LEDC_LS_CH_NUM; ch++) {
            ledc_pwm_set_duty_cycle(ledc_ls_ch, ch, 500);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        //低速LEDc，1通道，设置为固定占空比输出：100%（满分辨率为8191）
        printf("\t\t\t\t\t[3] LEDC LS_LED set duty = %d\%% without fade\n", 100);
        for (ch = 0; ch < LEDC_LS_CH_NUM; ch++) {
            ledc_pwm_set_duty_cycle(ledc_ls_ch, ch, 1000);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    //创建 ledc_pwm_hs_rgb3ch_task 任务，任务栈空间大小为 2048，任务优先级为3
    xTaskCreate(ledc_pwm_hs_rgb3ch_task, "ledc_pwm_hs_rgb3ch_task", 2048, NULL, 3, NULL);
    //创建 ledc_pwm_ls_led1ch_task 任务，运行任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(ledc_pwm_ls_led1ch_task, "ledc_pwm_ls_led1ch_task", 2048, NULL, 3, NULL);
}
