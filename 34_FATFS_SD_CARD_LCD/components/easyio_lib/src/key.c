
#include "key.h"

static const char *TAG = "KEY";
//按键跳变沿事件，的队列句柄（有抖动信息，不能直接使用。要使用经过二次处理的 key_evt_queue）
static xQueueHandle edge_evt_queue = NULL;
//按键触发事件，的队列句柄（过滤过按键抖动，存储着短按、长按信息事件）供外部应用调用。
// key_evt_queue队列接收到的消息类型为uint32_t，其中前两个字节为 按键类型（1短按，2长按），后两个为按键gpio序号，使用时需要拆分。
xQueueHandle key_evt_queue = NULL;

/**
 * @brief  定义 用户按键中断服务处理函数
 * 
 * @param  arg 传入触发中断的GPIO的Pin序号，用于检测哪个按键触发，单按键时无用
 * 
 * @return
 *     - none
 * 
 */
//定义 key_isr_handler 中断服务处理函数。IRAM_ATTR 是将函数定义在iRAM区
void IRAM_ATTR key_isr_handler(void *arg)
{
    // 读取gpio的Pin序号
    uint32_t gpio_num = (uint32_t) arg;
    // 读取按键电平状态，判断跳变沿类型。下降沿为0，上升沿为1
    uint32_t edge = gpio_get_level(gpio_num);

    // 将两个事件合并为uint32_t。前两个字节为 跳变沿类型，后两个为gpio序号
    edge = gpio_num | (edge << 16);
    xQueueSendFromISR(edge_evt_queue, &edge, NULL);
}

// KEY的按键事件结构体
struct m_Key_Event
{
    uint32_t edge; //边沿触发类型。uint32_t的前两个字节为 跳变沿类型，后两个为gpio序号。下降沿为0，上升沿为1。
    int64_t now_time; //记录本次触发的系统时间
    int64_t POSEDGE_time, NEGEDGE_time; //记录上升沿、下降沿触发时的系统时间
    int64_t Pulse_time; //低电平持续时间（按键按下的持续时间）
    uint32_t pressed_type; //按键事件类型。uint32_t的前两个字节为 按键事件类型，后两个为gpio序号。1是短按，2是长按。需要队列接收后拆分处理
};

/**
 * @brief  按键检测任务
 *     - 需要FreeRTOS单独创建一个任务，单独去运行这个按键检测任务。栈空间大小与 MAX_GPIO有关，建议起步4096
 *     - 用户需要另外创建队列接收任务，接收 key_evt_queue 队列的按键事件。
 *     - key_evt_queue队列接收到的消息类型为uint32_t，其中前两个字节为 按键类型（1短按，2长按），后两个为按键gpio序号，使用时需要拆分。
 *     - 如需关闭key串口调试开关，请将 key.h 中的 key_Dlog 设置为0
 *     - 对于本代码，电路使用按键直连GND。驱动中ESP32默认开启GPIO的上拉电阻。
 * 
 *     - 支持输入不定数目的GPIO通道参数，以便用一句函数实现多按键初始化。
 *     - 例：key_scan(1, KEY);
 *     - 例：key_scan(3, 12, 13, 14);
 * 
 * @param  key_num   总按键 的数量
 * @param  (...)    按键GPIO序号列表。支持不定数目参数，数目为 0~MAX_GPIO。值为 0, 1, 2... MAX_GPIO。
 * 
 * @return
 *     - none
 * 
 */
void key_scan(int key_num, ...)
{
    struct m_Key_Event key_Event[MAX_GPIO+1]; // MCU最大的GPIO序号。当然越大使用的栈空间就越大。

    uint32_t edge = 0; //边沿触发类型。下降沿为0，上升沿为1
    uint32_t key_gpio = 0; // 拆分出key对应的gpio按键值
    uint32_t pressed_type = 0; //最后合成的uint32_t，并通过队列发送到外部。前两个字节为 按键类型（1短按，2长按），后两个为按键gpio序号

    //创建一个长度为10的消息队列，用于接收 gpio isr中断处理服务
    //这个长度一定要大于 单个任务周期内，突发事件的发生次数，如超出会丢失部分消息
    //按键跳变沿事件队列（有抖动信息，不能直接使用。要使用二次处理过的 key_evt_queue）
    edge_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //按键触发事件队列（过滤过按键抖动，存储着短按、长按信息事件）供外部应用调用
    key_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    //由输入的不定数目参数，解析出端口值
    int channels[MAX_GPIO+1] = {0}; //解析出的端口列表
    va_list va;
    va_start(va, key_num); // init va, pointing to the first argument
    for(int i=0; i<key_num; i++) {
        channels[i] = va_arg(va,int); // get the next argument, the type is int
    }
    va_end(va);

    //配置 key 物理端口
    for(int i=0; i<key_num; i++) {
        //配置KEY_GPIO引脚为上拉输入、双边沿中断、关联ISR中断处理服务函数 key_isr_handler
        gpiox_set_intr_input(channels[i], GPIO_PULLUP_ENABLE, GPIO_PULLDOWN_DISABLE, GPIO_PIN_INTR_ANYEDGE, key_isr_handler);
    }
    
    //通过下降沿、上升沿的间隔时间来消除按键抖动，并判断出是短按、长按
    while(1) {
        //以阻塞方式，不断读取 edge_evt_queue 队列信息，监控按键跳变沿事件
		if (xQueueReceive(edge_evt_queue, &edge, portMAX_DELAY)) {
            //拆分按键事件，将按键值、跳变沿类型放入不同按键的结构体中
            key_gpio = edge & 0x0000FFFF; // 按键值
            key_Event[key_gpio].edge = edge >> 16; // 跳变沿类型

            //获取当前系统时间。计数1000是1ms，1s是1000000
            key_Event[key_gpio].now_time = esp_timer_get_time();
            //通过队列信息判断当前触发为 下降沿、或是上升沿
            if(key_Event[key_gpio].edge) {
                key_Event[key_gpio].POSEDGE_time = key_Event[key_gpio].now_time;//记录上升沿的系统时间
            }else{
                key_Event[key_gpio].NEGEDGE_time = key_Event[key_gpio].now_time;//记录下降沿的系统时间
            }
            
            //如果当前事件是上升沿，那么可能有两种情况：抖动、用户按键松开，此时进行判断
            //根据与上次下降沿的时间间隔（低电平持续时间），过滤掉小的抖动
            if(key_Event[key_gpio].edge) {
                key_Event[key_gpio].Pulse_time = key_Event[key_gpio].POSEDGE_time - key_Event[key_gpio].NEGEDGE_time;
                if(key_Event[key_gpio].Pulse_time > Jitter) { //滤除小于8ms的抖动
                    if((key_Event[key_gpio].Pulse_time > LongPress)) { //捕获大于1s，判定为长按
                        key_Event[key_gpio].pressed_type = 2;
                    }else{
                        key_Event[key_gpio].pressed_type = 1; //8ms < Pressed < 1s 区间内的，判定为短按
                    }
                    //合成携带按键值、按键类型的uint32_t
                    pressed_type = key_gpio | (key_Event[key_gpio].pressed_type << 16);
                    //向队列发送消息，用户可以在外部应用使用 key_evt_queue 来获取按键事件。
                    xQueueSend(key_evt_queue, &pressed_type, 10);
                    #if key_Dlog
                    //调试信息，打印 按键值、按键按下的时长（低电平持续时间）、按键类型（1为短按，2为长按）
                    ESP_LOGI(TAG, "Key:%d, Pulse: %lld ms, Type: %d\n", key_gpio, key_Event[key_gpio].Pulse_time/1000, key_Event[key_gpio].pressed_type);
                    #endif
                }else
                    key_Event[key_gpio].pressed_type = 0;
            }
            #if key_Dlog
            //调试信息，打印 按键值、按键触发的边沿类型（下降沿为0，上升沿为1）、系统时间
            ESP_LOGI(TAG, "Key:%d, Edge: %d, SYS_TIME: %lld\n", key_gpio, key_Event[key_gpio].edge, key_Event[key_gpio].now_time);
            #endif
        }
    }
}
