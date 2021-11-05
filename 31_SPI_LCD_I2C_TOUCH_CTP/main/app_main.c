
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

// 编译此代码不要用 `IDF-V4.3`，建议使用 `IDF-V4.2`。IDF-V4.3在编译I2C，运行后调试会出现许多莫名其妙的Bug，请等待乐鑫官方更新。
// 在IDF-V4.3中编译、下载运行会报错：i2c_param_config(645): i2c clock choice is invalid, please check flag and frequency
// i2c_set_pin(826): scl and sda gpio numbers are the same
// 而在 `IDF-V4.2` 中，是可以被正常编译、下载和运行的

// lcd_ctp_task 任务。初始化 SPI3-LCD，和电容触摸屏 FTxxxx。并在屏幕上显示触摸轨迹及信息。
void lcd_ctp_task(void* arg)
{
    // 配置SPI3-主机模式，配置DMA通道、DMA字节大小，及 MISO、MOSI、CLK的引脚。
    spi_master_init(SPI3_HOST, LCD_DEF_DMA_CHAN, LCD_DMA_MAX_SIZE, SPI3_DEF_PIN_NUM_MISO, SPI3_DEF_PIN_NUM_MOSI, SPI3_DEF_PIN_NUM_CLK);
    // lcd-驱动IC初始化（注意：普通GPIO最大只能30MHz，而IOMUX默认的SPI引脚，CLK最大可以设置到80MHz）（注意排线不要太长，高速时可能会花屏）
    spi_lcd_init(SPI3_HOST, 40*1000*1000, LCD_SPI3_DEF_PIN_NUM_CS0);

    // 清屏，用单一底色
    LCD_Clear(LGRAYBLUE);
    // 显示字符串
    LCD_ShowString(1-1,1-1,YELLOW,BLUE,"TP_NUM:",12,0);
    LCD_ShowString(1-1,20-1,YELLOW,BLUE,"X:",12,0);
    LCD_ShowString(1-1,32-1,YELLOW,BLUE,"Y:",12,0);
    LCD_ShowString(1-1,44-1,YELLOW,BLUE,"EVT:",12,0);
    LCD_ShowString(1-1,56-1,YELLOW,BLUE,"ID:",12,0);
    // 显示数字
    LCD_ShowNum(48-1,1-1,YELLOW,BLUE,0,1,12,0);

    // 触摸IC的信息结构体，方便管理触摸发生的5个点信息
    ctp_tp_t ctp;
    // 5个触摸点各自的颜色
    uint16_t tp_color[5] = {RED, GREEN, BLUE, YELLOW, WHITE};

    //gpiox_set_ppOutput(25, 1); // 如电容触摸屏不能正常初始化，建议加上此句，将RESET拉高。如仍不可以，则触摸屏硬件可能损坏
    //vTaskDelay(200 / portTICK_PERIOD_MS);
    // 配置I2C0-主机模式，400K，指定 SCL-14，SDA-4
    i2c_master_init(I2C_NUM_0, 400000, GPIO_NUM_14, GPIO_NUM_4);
    // FTxxxx触控芯片初始化
    i2c_ctp_FTxxxx_init(I2C_NUM_0);

    while(1) {
        // 读FTxxxx 所有触摸信息。（触摸点个数、坐标、事件、ID）
        i2c_ctp_FTxxxx_read_all(I2C_NUM_0, &ctp);

        // 显示触摸发生的总个数
        LCD_ShowNum(48-1,1-1,YELLOW,BLUE,ctp.tp_num,1,12,0);
        // 显示各点坐标及事件类型
        for (uint16_t i=0;i<ctp.tp_num;i++) {
            ctp.tp[i].x = ((int16_t)(ctp.tp[i].x - 160) - 319) * -1; // 去除`FT6236U`触摸屏的x坐标固有偏移，再对屏幕倒立的x轴进行补偿
            ctp.tp[i].y = ((int16_t)ctp.tp[i].y - 239) * -1; // 对屏幕倒立的y轴进行补偿
            LCD_ShowNum(40*i+30,20-1,tp_color[ctp.tp[i].id],BLACK,ctp.tp[i].x,5,12,0); // x坐标
            LCD_ShowNum(40*i+30,32-1,tp_color[ctp.tp[i].id],BLACK,ctp.tp[i].y,5,12,0); // y坐标
            LCD_ShowNum(40*i+30,44-1,tp_color[ctp.tp[i].id],BLACK,ctp.tp[i].event,5,12,0); // 事件类型：0-按下；1-抬起；2-接触/长按；3-无事件
            LCD_ShowNum(40*i+30,56-1,tp_color[ctp.tp[i].id],BLACK,ctp.tp[i].id,5,12,0); // 触摸ID

            // 在手指触摸的地方描点，显示触摸轨迹：
            //LCD_DrawPoint(ctp.tp[i].x, ctp.tp[i].y, tp_color[ctp.tp[i].id]); // 单点，显示效果不明显
            LCD_DrawFillRectangle(ctp.tp[i].x, ctp.tp[i].y, ctp.tp[i].x+1, ctp.tp[i].y+1, tp_color[ctp.tp[i].id]); // 每个点是一个2x2的矩形块，显示效果清晰
        }
        // 清空其他无触摸的点 
        LCD_DrawFillRectangle(40*ctp.tp_num+30, 20-1, 40*5+30, 56-1+12, LGRAYBLUE);
		vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 lcd_ctp_task 任务。
    xTaskCreate(lcd_ctp_task, "lcd_ctp_task", 2048, NULL, 3, NULL);
}
