#ifndef __SIMPLE_GUI_H__
#define __SIMPLE_GUI_H__

#include "spi_lcd.h"
#include <math.h>

// LCD重要参数集
typedef struct
{										    
	uint16_t width;			//LCD 宽度
	uint16_t height;		//LCD 高度
	uint16_t id;			//LCD ID
	uint8_t  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t  wramcmd;		//开始写gram指令
	uint16_t  setxcmd;		//设置x坐标指令
	uint16_t  setycmd;		//设置y坐标指令	 
}_lcd_dev;

/**
 * @brief LCD的扫描方式定义（扫描方向）
 */
typedef enum {
    L2R_U2D = 0,    // 从左到右,从上到下
    L2R_D2U,        // 从左到右,从下到上
    R2L_U2D,        // 从右到左,从上到下
    R2L_D2U,        // 从右到左,从下到上

    U2D_L2R,        //从上到下,从左到右
    U2D_R2L,        //从上到下,从右到左
    D2U_L2R,        //从下到上,从左到右
    D2U_R2L,        //从下到上,从右到左
} lcd_scan_type_t;

/**
 * @brief 大多数屏幕的安装方向定义（以手册为准）
 * 
 * @param 横屏 正向摆放
 * 
    +----------------------------+
    |                           ||
    |                           ||--------
    |                           ||-- 排 --
    |                           ||-- 线 --
    |                           ||--------
    |                           ||
    +----------------------------+

 * @param 横屏 倒立摆放
 * 
            +----------------------------+
            ||                           |
    --------||                           |
    -- 排 --||                           |
    -- 线 --||                           |
    --------||                           |
            ||                           |
            +----------------------------+


 * @param 竖屏 正向摆放
 * 
    +----------------+
    |                |
    |                |
    |                |
    |                |
    |                |
    |                |
    |                |
    +----------------+
         ||||||||
         ||排线||
         |||||||



 * @param 竖屏 倒立摆放
 * 
         ||||||||
         ||排线||
         |||||||
    +----------------+
    |                |
    |                |
    |                |
    |                |
    |                |
    |                |
    |                |
    +----------------+
    
 */

/**
 * @brief LCD安装的方向定义（设置横屏还是竖屏显示）
 */
typedef enum {
    vertical = 0,    // 竖屏
    horizontal,      // 横屏
} lcd_display_dir_t;

/**
 * @brief LCD安装的倒置定义（设置屏幕是正着摆放还是倒立摆放的）
 */
typedef enum {
    invert_dis = 0,    // 不翻转，屏幕正着摆放
    invert_en,         // 翻转，屏幕反着摆放
} lcd_display_invert_t;

/**
 * @brief LCD安装的镜像定义（可用于镜面反射及棱镜的镜像显示，可使画面左右翻转。参照分光棱镜）
 */
typedef enum {
    mirror_dis = 0,    // 不镜像，正常显示
    mirror_en,         // 镜像，可用于镜面反射及棱镜的镜像显示
} lcd_display_mirror_t;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	 0X841F //浅绿色
#define LIGHTGRAY        0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 函数的功能说明见 simple_gui.c
void LCD_Display_Resolution(lcd_display_dir_t dir);
void LCD_Display_Dir(lcd_display_dir_t dir, lcd_display_invert_t invert, lcd_display_mirror_t mirror);
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_Clear(uint16_t color);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawAngleLine(uint16_t x,uint16_t y,uint16_t angle,uint16_t r,uint16_t color);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawFillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawCircle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color);

void LCD_ShowChar(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,uint8_t ch,uint8_t size,uint8_t mode);
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,char *p,uint8_t size,uint8_t mode);
void LCD_ShowNum(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);
void LCD_ShowFloat(uint16_t x, uint16_t y,uint16_t bcolor, uint16_t fcolor, float num, uint8_t len, uint8_t size, uint8_t mode);
void LCD_ShowPicture_16b(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t *p);


#endif
