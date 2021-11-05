
#include "easyio.h"

#define LED 33
#define KEY 0
#define KEY_USR 27

static const char *TAG = "main";
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

// i2c_sht30_task 任务。初始化 SHT30工作于周期测量模式，获取环境温湿度数据
void i2c_sht30_task(void* arg)
{
    // 配置I2C0-主机模式，400K，指定 SCL-14，SDA-4
    i2c_master_init(I2C_NUM_0, 400000, GPIO_NUM_14, GPIO_NUM_4);

    uint8_t recv_dat[6] = {0};
    float temperature = 0.0;
    float humidity = 0.0;

    SHT30_reset(); // 复位SHT30
    if(SHT30_Init() == ESP_OK) // 初始化SHT30(周期测量模式)
        ESP_LOGI(TAG, "sht30 init ok.\n");
    else
        ESP_LOGE(TAG, "sht30 init fail.\n");

    while(1) {
        if(SHT30_Read_Dat(recv_dat) == ESP_OK) // 从SHT30读取一次数据（周期测量模式下）
        {
            // 将SHT30接收的6个字节数据进行CRC校验，并转换为温度值和湿度值
            if(SHT30_Dat_To_Float(recv_dat, &temperature, &humidity)==0)
            {
                ESP_LOGI(TAG, "temperature = %.2f ℃, humidity = %.2f %%\n", temperature, humidity);
            }
            else
            {
                ESP_LOGE(TAG, "crc check fail.\n");
            }
        }
        else
        {
            ESP_LOGE(TAG, "read data from sht30 fail.\n");
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 i2c_sht30_task 任务。初始化 SHT30工作于周期测量模式，获取环境温湿度数据
    xTaskCreate(i2c_sht30_task, "i2c_sht30_task", 2048, NULL, 3, NULL);
}
