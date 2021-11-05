
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

// key_scan_task 任务，单独运行一个任务，运行按键检测（滤除按键抖动、短按、长按）
void key_scan_task(void* arg)
{
    //按键检测任务
    //key_scan(1, KEY); // 1个按键
    key_scan(2, KEY, KEY_USR); // 2个按键
}

// key_catch_task 任务。去捕获按键事件，来手动重置PCF8563的RTC日期、时间。
void key_catch_task(void* arg)
{
    uint32_t key_event = 0;
    uint32_t key_gpio = 0; // 触发按键事件的按键gpio序号
    uint32_t key_type = 0; // 触发按键事件的类型

    while(1) {
        // 以阻塞方式，不断读取 key_evt_queue 队列信息，监控按键事件
        if (xQueueReceive(key_evt_queue, &key_event, portMAX_DELAY)) {
            // 拆分按键事件，得到按键值和按键类型
            key_gpio = key_event & 0x0000FFFF; // 按键值
            key_type = key_event >> 16; // 按键类型（1为短按，2为长按）

            ESP_LOGW(TAG, "key:%d, type:%d\n", key_gpio, key_type); // 输出按键事件

            if(key_gpio == KEY) {
                if(key_type == 1) { // 1，短按
                    _PCF8563_Time_Typedef reset_time;
                    reset_time.RTC_Hours = 12;
                    reset_time.RTC_Minutes = 0;
                    reset_time.RTC_Seconds = 0;
                    PCF8563_SetTime(PCF_Format_BIN, &reset_time); // 设置时间。时、分、秒。12:00:00。
                }else if(key_type == 2) { // 2，长按
                    PCF8563_Set_Times(PCF_Format_BIN, PCF_Century_20xx, 21, 10, 1, 5, 23, 59); // 设置日期及时间。2021年10月1日 星期5 23:59，等待1分钟可观察日期的切换效果。
                }
            }
        }
    }
}

// i2c_pcf8563_rtc_task 任务。初始化 使用I2C总线的RTC芯片-PCF8563，来进行离线低功耗计时
// PCF8563有万年历，可获得 世纪、年、月、日、星期、时、分、秒 等日期/时间消息
void i2c_pcf8563_rtc_task(void* arg)
{
    // 配置I2C0-主机模式，400K，指定 SCL-14，SDA-4
    i2c_master_init(I2C_NUM_0, 400000, GPIO_NUM_14, GPIO_NUM_4);

    _PCF8563_Date_Typedef rtc_date;
    _PCF8563_Time_Typedef rtc_time;
    uint8_t rtc_century = 0; // 世纪位。0-20xx年；1-19xx年
    uint8_t week[7][3] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    uint8_t check = PCF8563_Check();
    ESP_LOGW(TAG, "check = %d\r\n", check);

    // 设置初始日期、时间
    PCF8563_Set_Times(PCF_Format_BIN, PCF_Century_20xx, 21, 10, 1, 5, 12, 0); // 设置日期及时间。2021年10月1日 星期5 12:00

    // 在初始化PCF8563时，只需调用 PCF8563_Start(); 此api会自动根据IC运行模式，将 PCF8563 启动计数。而不用再使用 PCF8563_SetMode();
    PCF8563_Start();

    while(1) {
        if (!check) { // check位为0，表示I2C总线上存在 PCF8563设备，开始读日期、时间
            PCF8563_GetDate(PCF_Format_BIN, &rtc_century, &rtc_date);
            PCF8563_GetTime(PCF_Format_BIN, &rtc_time);

            if (rtc_century == 0) { // 判断世纪位。0-20xx年；1-19xx年
                // 显示日期。年、月、日
                ESP_LOGW(TAG, "20%02d-%d-%d %.3s\t", rtc_date.RTC_Years, rtc_date.RTC_Months, rtc_date.RTC_Days, week[rtc_date.RTC_WeekDays]);
            } else {
                // 显示日期。年、月、日
                ESP_LOGW(TAG, "19%02d-%d-%d %s\t", rtc_date.RTC_Years, rtc_date.RTC_Months, rtc_date.RTC_Days, week[rtc_date.RTC_WeekDays]);
            }
            // 显示时间。时、分、秒
            ESP_LOGW(TAG, "%02d:%02d:%02d\r\n", rtc_time.RTC_Hours, rtc_time.RTC_Minutes, rtc_time.RTC_Seconds);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 key_scan_task 任务，单独运行一个任务，运行按键检测（滤除按键抖动、短按、长按）
    xTaskCreate(key_scan_task, "key_scan_task", 4096, NULL, 3, NULL);
    // 创建 key_catch_task 任务，去捕获按键事件，来手动重置PCF8563的RTC日期、时间。
    xTaskCreate(key_catch_task, "key_catch_task", 4096, NULL, 2, NULL);

    // 创建 i2c_pcf8563_rtc_task 任务。初始化 使用I2C总线的RTC芯片-PCF8563，来进行离线低功耗计时
    xTaskCreate(i2c_pcf8563_rtc_task, "i2c_pcf8563_rtc_task", 2048, NULL, 3, NULL);
}
