
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

// 编译此代码不要用 IDF-V4.3，建议使用 IDF-V4.2。IDF-V4.3在编译I2C，运行后调试会出现许多莫名其妙的Bug，请等待乐鑫官方更新。
// 31_SPI_LCD_I2C_TOUCH_CTP 也会有同样的错误，而且只能在IDF-V4.2中运行无误。
// 在IDF-V4.3中，下面这句不能放到任务里，放里编译后下载运行会报错：i2c_param_config(645): i2c clock choice is invalid, please check flag and frequency
// i2c_set_pin(826): scl and sda gpio numbers are the same
struct m_AHT20 AHT20; 
// i2c_aht20_task 任务。初始化AHT20温湿度传感器，读取温湿度
void i2c_aht20_task(void* arg)
{
    // 配置I2C0-主机模式，400K，指定 SCL-14，SDA-4
    i2c_master_init(I2C_NUM_0, 400000, GPIO_NUM_14, GPIO_NUM_4);
    // AHT20温湿度传感器初始化
    AHT20.alive = !i2c_aht20_init(I2C_NUM_0);

    while(1) {
        if(AHT20.alive)// 如果AHT20传感器存在，读取温湿度数据
		{
			// 读取AHT20的 20Bit原始数据
			AHT20.flag = i2c_aht20_read_ht(I2C_NUM_0, AHT20.HT);
			// 实际标准单位转换
			aht20_standard_unit_conversion(&AHT20);
		}
        // 打印输出
		printf("alive: %d    flag: %d\tRH: %.2f %%\tTemp:%.2f ℃\n", AHT20.alive, AHT20.flag, AHT20.RH, AHT20.Temp);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 i2c_aht20_task 任务。
    xTaskCreate(i2c_aht20_task, "i2c_aht20_task", 2048, NULL, 3, NULL);
}
