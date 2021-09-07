
#include "easyio.h"

#define LED 33
#define KEY 0

static xQueueHandle gpio_evt_queue = NULL; //队列返回句柄
// 任务句柄，包含创建任务的所有状态，对任务的操作都通过操作任务句柄实现
TaskHandle_t led_task_Handler;

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

// 记录按键按下次数
uint32_t intr_num = 0;
// 定义 gpio isr 中断服务处理函数。IRAM_ATTR 是将函数定义在iRAM区
void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    // 读取gpio的Pin序号，并将消息发送到队列中
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    intr_num++;
}

// gpio_in_task 任务，创建GPIO中断队列，串口输出gpio中断次数、led_task任务运行状态
void gpio_in_task(void* arg)
{
    uint8_t gpio_num = 0;

    // 创建一个长度为10的消息队列，用于接收 gpio isr中断处理服务
    // 这个长度一定要大于 单个任务周期内，突发事件的发生次数，如超出会丢失部分消息
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // 配置KEY引脚为上拉输入、下降沿中断、关联ISR中断处理服务函数 gpio_isr_handler
    gpiox_set_intr_input(KEY, GPIO_PULLUP_ENABLE, GPIO_PULLDOWN_DISABLE, GPIO_PIN_INTR_NEGEDGE, gpio_isr_handler);

    while(1) {
        // 以阻塞方式，不断读取 gpio_evt_queue 队列，读取后将消息从队列中删除
		if (xQueueReceive(gpio_evt_queue, &gpio_num, portMAX_DELAY)) {
            // 暂停/继续运行任务 led_task
            if(eTaskGetState(led_task_Handler) != eSuspended) {
                vTaskSuspend(led_task_Handler); // 暂停任务，LED停止闪烁
            }else{
                vTaskResume(led_task_Handler); // 继续任务，LED继续闪烁
            }
            // 串口输出GPIO中断的引脚Pin号、GPIO中断次数、led_task任务运行状态
            printf("KEY Pressed, Pin: %d, Num: %d, LED_Task_State: %d\r\n", gpio_num, intr_num, eTaskGetState(led_task_Handler));
		}
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);
    // 创建 gpio_in_task 任务，任务栈空间大小为 2048，任务优先级为3。实测分配小于1024会导致开机反复重启
    xTaskCreate(gpio_in_task, "gpio_in_task", 2048, NULL, 3, NULL);
}
