
#include "touch_pad_button.h"

//touch pad 中断触发位，记录各通道的触摸事件（0空闲，1-触摸）
bool s_pad_activated[TOUCH_PAD_MAX];
//touch pad 的初始感应值，用来做中断阈值的较准（注意，在初始化过程中，电极请勿触碰其他物体，以免干扰此值的正常获取）
uint32_t s_pad_init_val[TOUCH_PAD_MAX];

// 设置touch pad中断阈值
// 通过读取 touch pad 的初始感应值，使用初始值的 2/3 作为阈值（注意，在初始化过程中，电极请勿触碰其他物体）
static void tp_example_set_thresholds(void)
{
    uint16_t touch_value;
    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
        //read filtered value
        touch_pad_read_filtered(i, &touch_value);
        s_pad_init_val[i] = touch_value;
        //ESP_LOGI(TAG, "test init: touch pad [%d] val is %d", i, touch_value);
        //set interrupt threshold.
        ESP_ERROR_CHECK(touch_pad_set_thresh(i, touch_value * 2 / 3));

    }
}

//触摸ISR中断服务处理函数
//并将各通道触摸中断事件解析为 s_pad_activated 列表（0空闲，1-触摸）
static void tp_example_rtc_intr(void *arg)
{
    uint32_t pad_intr = touch_pad_get_status();
    // 清除中断
    touch_pad_clear_status();
    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
        if ((pad_intr >> i) & 0x01) {
            s_pad_activated[i] = true;
        }
    }
}

/*  ESP32 Touch_Pad 与 Pin 对照表

    +----------+--------------------+------------------+
    |          |    touch signal    |    Pin / Name    |
    +==========+====================+==================+
    |          |         T0         |      GPIO4       |
    |          +--------------------+------------------+
    |          |         T1         |      GPIO0  !    |
    |          +--------------------+------------------+
    |          |         T2         |      GPIO2       |
    |          +--------------------+------------------+
    |          |         T3         |    MTDO/GPIO15   |
    |  ESP32   +--------------------+------------------+
    |  touch   |         T4         |    MTCK/GPIO13   |
    |  sensor  +--------------------+------------------+
    |          |         T5         |    MTDI/GPIO12   |
    |          +--------------------+------------------+
    |          |         T6         |    MTMS/GPIO14   |
    |          +--------------------+------------------+
    |          |         T7         |      GPIO27      |
    |          +--------------------+------------------+
    |          |         T8         |   32K_XN/GPIO33  |
    |          +--------------------+------------------+
    |          |         T9         |   32K_XP/GPIO32  |
    +----------+--------------------+------------------+

*/

/**
 * @brief  触摸板按键检测初始化
 *      - 支持输入不定数目的通道参数，总参数数目为 2~11。
 *        例：
 *          touch_pad_button_init(1, TOUCH_PAD_NUM0);
 *          touch_pad_button_init(4, TOUCH_PAD_NUM0, TOUCH_PAD_NUM2, TOUCH_PAD_NUM3, TOUCH_PAD_NUM4);
 *      - ESP32 的T1为GPIO0，一般有电阻下拉到GND做BOOT模式选择，导致触摸失效，所以尽量不要用T1做 touch pad 功能
 * 
 * @param  tp_num   总触摸按键 的数量
 * @param  (...)    触摸通道列表。支持不定数目参数，数目为 1~10。值为 TOUCH_PAD_NUM0, TOUCH_PAD_NUM1... TOUCH_PAD_NUM9。
 * 
 * @return
 *      - none
 * 
 */
void touch_pad_button_init(int tp_num, ...)
{
    //初始化touch_pad外设
    touch_pad_init();
    //使用中断触发模式
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    //设定充放电参考电压，高参考电压将为2.7V-1V = 1.7V,低参考电压为0.5。电压范围越大，脉冲计数值越多。
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    
    //由输入的不定数目参数，解析出端口值
    int channels[10] = {0}; //解析出的端口列表
    va_list va;
    va_start(va, tp_num); // init va, pointing to the first argument
    for(int i=0; i<tp_num; i++) {
        channels[i] = va_arg(va,int); // get the next argument, the type is int
    }
    va_end(va);

    //配置 touch pad 物理端口
    for(int i=0; i<tp_num; i++) {
        //配置 touch pad 物理端口
        touch_pad_config(channels[i], TOUCH_THRESH_NO_USE);
    }

    // 启动软件滤波器
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    // 设置中断阈值
    tp_example_set_thresholds();
    // 注册触摸中断ISR中断服务处理函数
    touch_pad_isr_register(tp_example_rtc_intr, NULL);
    // Start a task to show what pads have been touched
    //xTaskCreate(&tp_example_read_task, "touch_pad_read_task", 2048, NULL, 5, NULL);

    //使能触摸中断
    touch_pad_intr_enable();
    // 清除触摸传感器通道的活动状态
    touch_pad_clear_status();
}
