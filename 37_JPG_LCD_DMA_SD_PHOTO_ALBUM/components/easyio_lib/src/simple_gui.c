#include "simple_gui.h"
#include "font.h"

//管理LCD重要参数
_lcd_dev lcddev;


/*//////////////////////////////////////////////////// 硬件SPI底层总线适配层 ////////////////////////////////////////////////////*/

// SPI操作LCD寄存器函数，写命令（固定长度，LCD的寄存器地址都是单字节-8Bits）
// 原型为 "spi_lcd.c" 中 void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd)
static void LCD_WR_REG(uint8_t reg_addr)
{
    lcd_cmd(LCD_SPI, reg_addr);
}

// SPI写LCD数据函数，写数据（可变长度）
// 原型为 "spi_lcd.c" 中 void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
static void LCD_WR_DATA(const uint8_t *data, int len)
{
    lcd_data(LCD_SPI, data, len);
}

// SPI向LCD发送单点16Bit的像素数据（像素长度固定，但根据驱动IC的不同可能是RGB565分16Bit-2字节，也可能为RGB666-18Bit分3字节传输）
// 原型为 "spi_lcd.c" 中 void lcd_data16(spi_device_handle_t spi, uint16_t data)
static void LCD_WR_DATA16(uint16_t data)
{
    lcd_data16(LCD_SPI, data);
}

// 写LCD寄存器，写单字节命令
static void LCD_WriteReg(uint8_t LCD_Reg, uint8_t LCD_RegValue)
{
	LCD_WR_REG(LCD_Reg);
	LCD_WR_DATA(&LCD_RegValue, 1);
}

// 开始写GRAM，命令（0x2C）
static void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(0x2C);
}

/*//////////////////////////////////////////////////// LCD驱动IC的功能接口层 ////////////////////////////////////////////////////*/

/**
 * @brief  设置光标位置（写入窗口大小为 当前光标~全屏）
 * 		- 若驱动IC为 stxxxx，需要到此函数修改GRAM偏移基地址。ili没有偏移地址。
 * 
 * @param  Xpos 横坐标（像素数）
 * @param  Ypos 纵坐标（像素数）
 * 
 * @return
 *     - none
 */
static void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
    uint8_t databuf[4] = {0};
	uint8_t xBase=0,yBase=0;

#ifdef CONFIG_LCD_TYPE_ST7735
	// 如果驱动IC为 ST7735，则需要增加偏移基地址。横屏：x-3，y-2
	xBase = 3;
	yBase = 2;
#elif defined( CONFIG_LCD_TYPE_ST7735S )
	// 如果驱动IC为 ST7735，则需要增加偏移基地址。横屏：x-0，y-24
	xBase = 0;
	yBase = 24;
#endif
	// ili系列没有偏移基地址
	databuf[0] = (Xpos+xBase)>>8;
	databuf[1] = (Xpos+xBase)&0XFF;
	databuf[2] = (lcddev.width-1+xBase)>>8;
	databuf[3] = (lcddev.width-1+xBase)&0XFF;

	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(databuf,4);

	databuf[0] = (Ypos+yBase)>>8;
	databuf[1] = (Ypos+yBase)&0XFF;
	databuf[2] = (lcddev.height-1+yBase)>>8;
	databuf[3] = (lcddev.height-1+yBase)&0XFF;

	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(databuf,4);
}

/**
 * @brief  设置写入窗口大小，并设置写入光标的位置
 * 		- 区别于上一句 LCD_SetCursor。LCD_SetWindows更为灵活。（这种方式在需要局部刷新，例如操作字符取模的写入上更加高效）
 * 		- 若驱动IC为 stxxxx，需要到此函数修改GRAM偏移基地址。ili没有偏移地址。
 * 
 * @param  xStar/xEnd 设置列的起始和结束地址
 * @param  yStar/yEnd 设置行的起始和结束地址
 * 
 * @return
 *     - none
 */
static void LCD_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd)
{
	uint8_t databuf[4] = {0};
#ifdef CONFIG_LCD_TYPE_ST7735
	// 如果驱动IC为 ST7735，则需要增加偏移基地址。横屏：x-3，y-2
	xStar += 3; // 加偏移基地址
	xEnd += 3;  // 加偏移基地址
	yStar += 2; // 加偏移基地址
	yEnd += 2;  // 加偏移基地址
#elif defined( CONFIG_LCD_TYPE_ST7735S )
	// 如果驱动IC为 ST7735S，则需要增加偏移基地址。横屏：x-0，y-24
	xStar += 0; // 加偏移基地址
	xEnd += 0;  // 加偏移基地址
	yStar += 24; // 加偏移基地址
	yEnd += 24;  // 加偏移基地址
#endif
	// ili系列没有偏移基地址
	databuf[0] = xStar>>8;
	databuf[1] = 0xFF&xStar;	
	databuf[2] = xEnd>>8;
	databuf[3] = 0xFF&xEnd;
	LCD_WR_REG(0x2A);
	LCD_WR_DATA(databuf,4);

	databuf[0] = yStar>>8;
	databuf[1] = 0xFF&yStar;	
	databuf[2] = yEnd>>8;
	databuf[3] = 0xFF&yEnd;
	LCD_WR_REG(0x2B);
	LCD_WR_DATA(databuf,4);

	LCD_WR_REG(0x2C);	//开始写入GRAM
}

/**
 * @brief  设置LCD的自动扫描方向（寄存器地址0x36）
 * 		- 修改LCD扫描方向的目的是，以适配屏幕的不同安装方向。
 * 		- 若驱动IC为 stxxxx，修改扫描方向后会影响到 LCD_SetCursor、LCD_SetWindows 中的GRAM偏移基地址
 * 
 * @param  dir LCD的安装方向。L2R_U2D/L2R_D2U/R2L_U2D/R2L_D2U/U2D_L2R/U2D_R2L/D2U_L2R/D2U_R2L,代表8个方向(包括左右镜像翻转)
 * 
 * @return
 *     - none
 */
static void LCD_Scan_Dir(uint8_t dir)
{
	uint8_t regval=0;
	uint8_t dirreg=0;
	uint16_t temp;
	
	// 0x36寄存器，设置扫描方向和方式
	switch(dir)
	{
			case L2R_U2D://从左到右,从上到下
				regval|=(0<<7)|(0<<6)|(0<<5); 
				break;
			case L2R_D2U://从左到右,从下到上
				regval|=(1<<7)|(0<<6)|(0<<5); 
				break;
			case R2L_U2D://从右到左,从上到下
				regval|=(0<<7)|(1<<6)|(0<<5); 
				break;
			case R2L_D2U://从右到左,从下到上
				regval|=(1<<7)|(1<<6)|(0<<5); 
				break;	 
			case U2D_L2R://从上到下,从左到右
				regval|=(0<<7)|(0<<6)|(1<<5); 
				break;
			case U2D_R2L://从上到下,从右到左
				regval|=(0<<7)|(1<<6)|(1<<5); 
				break;
			case D2U_L2R://从下到上,从左到右
				regval|=(1<<7)|(0<<6)|(1<<5); 
				break;
			case D2U_R2L://从下到上,从右到左
				regval|=(1<<7)|(1<<6)|(1<<5); 
				break;
	}
	dirreg=0X36;
#if CONFIG_LCD_TYPE_ST7789V
 	regval|=0X00;//RGB
	// 翻转Bit7
	if (regval&0x80) { // 如果Bit7=1，清0
		regval &= 0x7F;
	} else { // 如果Bit7=0，置1
		regval |= 0x80;
	}
#elif ( CONFIG_LCD_TYPE_ST7735 || CONFIG_LCD_TYPE_ST7735S)
	regval|=0X08;//BGR
	// 翻转Bit7
	if (regval&0x80) { // 如果Bit7=1，清0
		regval &= 0x7F;
	} else { // 如果Bit7=0，置1
		regval |= 0x80;
	}
#else
	// ili系列屏幕
	regval|=0X08;//BGR
#endif
	LCD_WriteReg(dirreg, regval);

 	if((regval&0X20)||lcddev.dir==1)
	{
		if(lcddev.width<lcddev.height)//交换X,Y
		{
		  temp=lcddev.width;
			lcddev.width=lcddev.height;
			lcddev.height=temp;
 		}
		}else
		{
			if(lcddev.width>lcddev.height)//交换X,Y
			{
				temp=lcddev.width;
				lcddev.width=lcddev.height;
				lcddev.height=temp;
 			}
		}

		LCD_SetWindows(0, 0, (lcddev.width-1), (lcddev.height-1));
}

/**
 * @brief  设置LCD的横竖屏方向，并根据型号匹配像素数目（显示范围）
 * 		- 修改LCD扫描方向的目的是，以适配屏幕的不同安装方向。
 * 		- 若驱动IC为 stxxxx，修改扫描方向后会影响到 LCD_SetCursor、LCD_SetWindows 中的GRAM偏移基地址
 * 
 * @param  dir LCD的安装方向是竖屏还是横屏。vertical / horizontal
 * 
 * @return
 *     - none
 */
void LCD_Display_Resolution(lcd_display_dir_t dir)
{
	// 设置写GRAM命令、设置X/Y坐标命令
	// 并根据LCD的安装方向，来定义GUI的显示范围（分辨率）
	if(dir == vertical) // 竖屏
	{
		lcddev.dir=0; // 竖屏
		lcddev.wramcmd=0X2C;
	 	lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;
		// 根据屏幕硬件像素数目，及安装方向，来定义GUI的显示范围（分辨率）
#ifdef LCD_PIXEL_SIZE_80_160
		lcddev.width=80;
		lcddev.height=160;
#elif defined (LCD_PIXEL_SIZE_128_128)
		lcddev.width=128;
		lcddev.height=128;
#elif defined (LCD_PIXEL_SIZE_135_240)
		lcddev.width=135;
		lcddev.height=240;
#elif defined (LCD_PIXEL_SIZE_240_240)
		lcddev.width=240;
		lcddev.height=240;
#elif defined (LCD_PIXEL_SIZE_240_320)
		lcddev.width=240;
		lcddev.height=320;
#elif defined (LCD_PIXEL_SIZE_320_480)
		lcddev.width=320;
		lcddev.height=480;
#endif
	}else 				//横屏
	{
		lcddev.dir=1; // 横屏
		lcddev.wramcmd=0X2C;
	 	lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;
		// 根据屏幕硬件像素数目，及安装方向，来定义GUI的显示范围（分辨率）
#ifdef LCD_PIXEL_SIZE_80_160
		lcddev.width=160;
		lcddev.height=80;
#elif defined (LCD_PIXEL_SIZE_128_128)
		lcddev.width=128;
		lcddev.height=128;
#elif defined (LCD_PIXEL_SIZE_135_240)
		lcddev.width=240;
		lcddev.height=135;
#elif defined (LCD_PIXEL_SIZE_240_240)
		lcddev.width=240;
		lcddev.height=240;
#elif defined (LCD_PIXEL_SIZE_240_320)
		lcddev.width=320;
		lcddev.height=240;
#elif defined (LCD_PIXEL_SIZE_320_480)
		lcddev.width=480;
		lcddev.height=320;
#endif
	}
}

/**
 * @brief  设置LCD的显示方向
 * 		- 根据 横竖屏、是否倒置、是否镜像 去匹配LCD的扫描方向
 * 		- 若驱动IC为 stxxxx，修改扫描方向后会影响到 LCD_SetCursor、LCD_SetWindows 中的GRAM偏移基地址
 * 
 * @param  dir LCD的安装方向是竖屏还是横屏。vertical / horizontal
 * @param  invert LCD是否倒置。invert_dis / invert_en
 * @param  mirror LCD是否镜像安装。mirror_dis / mirror_en（可用于镜面反射及棱镜的镜像显示，可使画面左右翻转。参照分光棱镜）
 * 
 * @return
 *     - none
 */
void LCD_Display_Dir(lcd_display_dir_t dir, lcd_display_invert_t invert, lcd_display_mirror_t mirror)
{
	lcd_scan_type_t scan_type = 0;
	// 设置屏幕扫描方向，来匹配屏幕的安装方向。或镜像安装方式（可用于镜面反射及棱镜的镜像显示，例如分光棱镜及镜面反射显示）
	if ((dir == vertical) && (invert == invert_dis) && (mirror == mirror_dis)) {          // 竖屏、不翻转(正着摆放)、不镜像
		scan_type = R2L_U2D; // 竖屏，从左到右，从上到下
	} else if ((dir == vertical) && (invert == invert_dis) && (mirror == mirror_en)) {    // 竖屏、不翻转(正着摆放)、镜像
		scan_type = L2R_U2D; // 竖屏，从右到左，从上到下
	} else if ((dir == vertical) && (invert == invert_en) && (mirror == mirror_dis)) {    // 竖屏、翻转(反着摆放)、不镜像
		scan_type = L2R_D2U; // 竖屏，从左到右，从下到上
	} else if ((dir == vertical) && (invert == invert_en) && (mirror == mirror_en)) {     // 竖屏、翻转(反着摆放)、镜像
		scan_type = R2L_D2U; // 竖屏，从右到左，从下到上
	} else if ((dir == horizontal) && (invert == invert_dis) && (mirror == mirror_dis)) {    // 横屏、不翻转(正着摆放)、不镜像
		scan_type = U2D_L2R; // 横屏，从上到下，从左到右
	} else if ((dir == horizontal) && (invert == invert_dis) && (mirror == mirror_en)) {     // 横屏、不翻转(正着摆放)、镜像
		scan_type = D2U_L2R; // 横屏，从上到下，从右到左
	} else if ((dir == horizontal) && (invert == invert_en) && (mirror == mirror_dis)) {     // 横屏、翻转(反着摆放)、不镜像
		scan_type = D2U_R2L; // 横屏，从下到上，从左到右
	} else if ((dir == horizontal) && (invert == invert_en) && (mirror == mirror_en)) {      // 横屏、翻转(反着摆放)、镜像
		scan_type = U2D_R2L; // 横屏，从下到上，从右到左
	}
	LCD_Scan_Dir(scan_type);    // 根据屏幕的不同安装方式，来适配不同的扫描方向
}

/*//////////////////////////////////////////////////// GUI绘图的应用层 ////////////////////////////////////////////////////*/

/**
 * @brief  GUI画点函数
 * 
 * @param  x 横坐标（像素数，从0开始）
 * @param  y 纵坐标（像素数，从0开始）
 * @param  color 画点的颜色，uint16_t - RGB565编码
 * 
 * @return
 *     - none
 */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
	LCD_SetCursor(x,y);//设置光标位置 
	LCD_WR_REG(0X2C);//开始写入GRAM
	LCD_WR_DATA16(color);
}

/*//清屏函数（因为每次SPI传输只刷一个点，而ESP32的SPI默认开启DMA，每发一个像素都会重入DMA，所以非常慢，低效）
//color:要清屏的填充色
void LCD_Clear(uint16_t color)
{
	uint32_t index=0;
	uint32_t totalpoint=lcddev.width;
	totalpoint*=lcddev.height; 	//得到总点数
	
	LCD_SetCursor(0x00,0x0000);	//设置光标位置
	LCD_WriteRAM_Prepare();     //开始写入GRAM
	for(index=0;index<totalpoint;index++)
	{
		LCD_WR_DATA16(color);
	}
}*/

uint8_t lineBuf[LINE_PIXEL_MAX_SIZE*3] = {0}; // 行像素内容缓存。不但可以作为清屏使用，还可以在刷图时使用，提高效率

/**
 * @brief  GUI清屏函数
 * 		- 高效率版本，使用行缓存按行来刷点，减少重入DMA传输的次数，大幅度提高刷屏速度
 * 		- 当然可以通过增大行缓存容量，来继续减少重入DMA次数，可为了适配不同分辨率的屏幕，每次只刷1行是最方便的
 * 
 * @param  color 清屏填充的颜色，uint16_t - RGB565编码
 * 
 * @return
 *     - none
 */
void LCD_Clear(uint16_t color)
{
	uint32_t i=0;

#if !(CONFIG_LCD_TYPE_ILI9488 || CONFIG_LCD_TYPE_ILI9481)
	// 计算单行像素缓存内容。16Bit/像素，2字节/像素
	lineBuf[0] = color>>8; // 高8位
	lineBuf[1] = color; // 低8位
	for(i=1; i<lcddev.width; i++){
		lineBuf[i*2] = lineBuf[0];
		lineBuf[i*2+1] = lineBuf[1];
	}
#else
    // ili9488用SPI总线驱动，像素必须为18Bit/pixel，16Bit会没有显示。（需要提前设置0x3A寄存器）
	// 计算单行像素缓存内容。18Bit/像素，3字节/像素
	lineBuf[0] = (color>>8)&0xF8;//RED
	lineBuf[1] = (color>>3)&0xFC;//GREEN
	lineBuf[2] = color<<3;//BLUE
	for(i=1; i<lcddev.width; i++){
		lineBuf[i*3] = lineBuf[0];
		lineBuf[i*3+1] = lineBuf[1];
		lineBuf[i*3+2] = lineBuf[2];
	}
#endif

	// 设置光标，开始刷屏
	LCD_SetCursor(0x00,0x0000);	//设置光标位置
	LCD_WriteRAM_Prepare();     //开始写入GRAM

	// 将单行像素缓存，刷新到LCD每一行。每次刷一行，按行刷
	for(i=0; i<lcddev.height; i++)
	{
#if !(CONFIG_LCD_TYPE_ILI9488 || CONFIG_LCD_TYPE_ILI9481)
		lcd_data(LCD_SPI, lineBuf, lcddev.width*2);
#else
		lcd_data(LCD_SPI, lineBuf, lcddev.width*3);
#endif
	}
}

/**
 * @brief  GUI画线函数
 * 
 * @param  x1 起点横坐标
 * @param  y1 起点纵坐标
 * @param  x2 终点横坐标
 * @param  y1 终点纵坐标
 * @param  color 画线填充的颜色，uint16_t - RGB565编码
 * 
 * @return
 *     - none
 */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1;
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y;
	for(t=0;t<=distance+1;t++ )//画线输出
	{
		LCD_DrawPoint(uRow,uCol,color);//画点
		xerr+=delta_x ;
		yerr+=delta_y ;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

/**
 * @brief  GUI画角度线
 * 
 * @param  x 起点横坐标
 * @param  y 起点纵坐标
 * @param  angle 角度。0~360
 * @param  r 线长
 * @param  color 线的颜色，uint16_t - RGB565编码
 * 
 * @return
 *     - none
 */
#define pi 3.1415926535898
void LCD_DrawAngleLine(uint16_t x,uint16_t y,uint16_t angle,uint16_t r,uint16_t color)
{
	uint16_t x1,y1;
	x1=x+(float)r*sin((pi/180)*angle); 
	y1=y+(float)r*cos((pi/180)*angle); 
	LCD_DrawLine(x,y,x1,y1,color);
}

/**
 * @brief  GUI画空心矩形函数
 * 
 * @param  x1 起点横坐标
 * @param  y1 起点纵坐标
 * @param  x2 终点横坐标
 * @param  y1 终点纵坐标
 * @param  color 空心矩形四边的颜色，uint16_t - RGB565编码
 * 
 * @return
 *     - none
 */
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}

/**
 * @brief  GUI画实心矩形函数
 * 		- 高效率版本，使用行缓存按行来刷点，减少重入DMA传输的次数，大幅度提高刷屏速度
 * 		- 当然可以通过增大行缓存容量，来继续减少重入DMA次数，可为了适配不同分辨率的屏幕，每次只刷1行是最方便的
 * 
 * @param  x1 起点横坐标
 * @param  y1 起点纵坐标
 * @param  x2 终点横坐标
 * @param  y1 终点纵坐标
 * @param  color 实心矩形填充的颜色，uint16_t - RGB565编码
 * 
 * @return
 *     - none
 */
void LCD_DrawFillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint32_t i=0;

#if !(CONFIG_LCD_TYPE_ILI9488 || CONFIG_LCD_TYPE_ILI9481)
	// 计算单行像素缓存内容。16Bit/像素，2字节/像素
	lineBuf[0] = color>>8; // 高8位
	lineBuf[1] = color; // 低8位
	for(i=1; i<(x2-x1+1); i++){
		lineBuf[i*2] = lineBuf[0];
		lineBuf[i*2+1] = lineBuf[1];
	}
#else
    // ili9488用SPI总线驱动，像素必须为18Bit/pixel，16Bit会没有显示。（需要提前设置0x3A寄存器）
	// 计算单行像素缓存内容。18Bit/像素，3字节/像素
	lineBuf[0] = (color>>8)&0xF8;//RED
	lineBuf[1] = (color>>3)&0xFC;//GREEN
	lineBuf[2] = color<<3;//BLUE
	for(i=1; i<(x2-x1+1); i++){
		lineBuf[i*3] = lineBuf[0];
		lineBuf[i*3+1] = lineBuf[1];
		lineBuf[i*3+2] = lineBuf[2];
	}
#endif

	// 设置光标和写入窗口大小，开始刷屏
	LCD_SetWindows(x1,y1,x2,y2);  // 设置写入窗口大小，并设置写入光标的位置
	LCD_WriteRAM_Prepare();     //开始写入GRAM

	// 将单行像素缓存，刷新到矩形区域的每一行。每次刷一行，按行刷
	for(i=0; i<(y2-y1+1); i++)
	{
#if !(CONFIG_LCD_TYPE_ILI9488 || CONFIG_LCD_TYPE_ILI9481)
		lcd_data(LCD_SPI, lineBuf, (x2-x1+1)*2);
#else
		lcd_data(LCD_SPI, lineBuf, (x2-x1+1)*3);
#endif
	}
}

/**
 * @brief  GUI画空心圆
 * 
 * @param  x0 原点横坐标
 * @param  y0 原点纵坐标
 * @param  r 半径大小
 * @param  color 圆周的颜色，uint16_t - RGB565编码
 * 
 * @return
 *     - none
 */
void LCD_DrawCircle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color)
{
	int a,b;
	int di;
	a=0;b=r;
	di=3-(r<<1); // 判断下个点位置的标志
	while(a<=b)
	{
		LCD_DrawPoint(x0+b,y0-a,color);    //0
		LCD_DrawPoint(x0-a,y0+b,color);    //1
		LCD_DrawPoint(x0-b,y0-a,color);    //7
		LCD_DrawPoint(x0-a,y0-b,color);    //2
		LCD_DrawPoint(x0+b,y0+a,color);    //4
		LCD_DrawPoint(x0+a,y0-b,color);    //5
		LCD_DrawPoint(x0+a,y0+b,color);    //6
		LCD_DrawPoint(x0-b,y0+a,color);
		a++;
		//使用Bresenham算法画圆
		if(di<0)di +=4*a+6;
		else
		{
			di+=10+4*(a-b);
			b--;
		}
	}
}

/*//显示一个字符（按点刷，比较低效）
//x:0~234
//y:0~308
//bcolor:背景色
//fcolor:字体颜色
//ch:要显示的字符:" "--->"~"
//size:字体大小 12/16
//mode:非叠加方式，显示背景色(0)；叠加方式，不显示背景色(1)
void LCD_ShowChar(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,uint8_t ch,uint8_t size,uint8_t mode)
{  
#if USE_HORIZONTAL==1
#define MAX_CHAR_POSX 392
#define MAX_CHAR_POSY 232 
#else     
#define MAX_CHAR_POSX 232
#define MAX_CHAR_POSY 392
#endif 
    uint8_t temp;
    uint8_t pos,t;
	uint16_t x0=x;
    if(x>MAX_CHAR_POSX||y>MAX_CHAR_POSY)return;
	//设置窗口
	ch=ch-' ';//得到偏移后的值
    LCD_SetCursor(x, y);//移动光标
    LCD_WR_REG(0X2C);//开始写入GRAM
	if(!mode) //非叠加方式
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[ch][pos];//调用1206字体
			else temp=asc2_1608[ch][pos];		 //调用1608字体
			for(t=0;t<size/2;t++)
		    {
		        if(temp&0x01) LCD_DrawPoint(x,y,fcolor);
				else LCD_DrawPoint(x,y,bcolor);
				temp>>=1;
				x++;
		    }
			x=x0;
			y++;
		}
	}else//叠加方式
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[ch][pos];//调用1206字体
			else temp=asc2_1608[ch][pos];		 //调用1608字体
			for(t=0;t<size/2;t++)
		    {
		        if(temp&0x01)LCD_DrawPoint(x+t,y+pos,fcolor);//画一个点
		        temp>>=1;
		    }
		}
	}
}*/

/**
 * @brief  GUI显示单个字符
 * 		- 设置写入窗口大小，再写入数据（这种方式在操作字符取模的写入上，相比按点刷更加高效）
 * 
 * @param  x 起点横坐标
 * @param  y 起点纵坐标
 * @param  bcolor 背景色。background color
 * @param  fcolor 字体颜色。font color
 * @param  ch 显示的字符
 * @param  size 字体大小。12（1206） / 16（1608）
 * @param  mode 覆盖方式，显示背景色(0)；叠加方式，不显示背景色(1)
 * 
 * @return
 *     - none
 */
void LCD_ShowChar(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,uint8_t ch,uint8_t size,uint8_t mode)
{
	uint8_t temp,pos,t;
	ch=ch-' ';//得到偏移后的值
	LCD_SetWindows(x,y,x+size/2-1,y+size-1);//设置单个文字显示窗口
	if(!mode){ //非叠加方式
		for(pos=0;pos<size;pos++){
			if(size==12)temp=asc2_1206[ch][pos];//调用1206字体
			else temp=asc2_1608[ch][pos];		 //调用1608字体
			for(t=0;t<size/2;t++){
				if(temp&0x01)LCD_WR_DATA16(fcolor);
				else LCD_WR_DATA16(bcolor);
				temp>>=1;
			}
		}
	}else{//叠加方式
		for(pos=0;pos<size;pos++){
			if(size==12)temp=asc2_1206[ch][pos];//调用1206字体
			else temp=asc2_1608[ch][pos];		 //调用1608字体
			for(t=0;t<size/2;t++){
				if(temp&0x01)LCD_DrawPoint(x+t,y+pos,fcolor);//画一个点
				temp>>=1;
			}
		}
	}
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//恢复窗口为全屏
}

/**
 * @brief  GUI显示字符串
 * 
 * @param  x 起点横坐标
 * @param  y 起点纵坐标
 * @param  bcolor 背景色。background color
 * @param  fcolor 字体颜色。font color
 * @param  p 显示的字符串起始指针
 * @param  size 字体大小。12（1206） / 16（1608）
 * @param  mode 覆盖方式，显示背景色(0)；叠加方式，不显示背景色(1)
 * 
 * @return
 *     - none
 */
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,char *p,uint8_t size,uint8_t mode)
{
	while((*p<='~')&&(*p>=' ')){//判断是不是非法字符!
		if(x>(lcddev.width-1)||y>(lcddev.height-1)) 
		return;
		LCD_ShowChar(x,y,bcolor,fcolor,*p,size,mode);
		x+=size/2;
		p++;
	}
}


// m的n次方
static uint32_t mypow(uint8_t m, uint8_t n)
{
	uint32_t result=1;
	while(n--)result*=m;
	return result;
}

/**
 * @brief  GUI显示整形数字（空余位有空格显示）
 * 
 * @param  x 起点横坐标
 * @param  y 起点纵坐标
 * @param  bcolor 背景色。background color
 * @param  fcolor 字体颜色。font color
 * @param  num 显示的整形数字
 * @param  len 数字的长度
 * @param  size 字体大小。12（1206） / 16（1608）
 * @param  mode 覆盖方式，显示背景色(0)；叠加方式，不显示背景色(1)
 * 
 * @return
 *     - none
 */
void LCD_ShowNum(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
{
	uint8_t t,temp,enshow=0;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,bcolor,fcolor,' ',size,mode);
				continue;
			}else enshow=1;
		}
	 	LCD_ShowChar(x+(size/2)*t,y,bcolor,fcolor,temp+'0',size,mode);
	}
}

/**
 * @brief  GUI显示浮点数字
 * 		- 限制了最大显示的小数点位数为后5位，如需更长请自行更改代码
 * 
 * @param  x 起点横坐标
 * @param  y 起点纵坐标
 * @param  bcolor 背景色。background color
 * @param  fcolor 字体颜色。font color
 * @param  num 显示的浮点数字
 * @param  len 数字的长度
 * @param  size 字体大小。12（1206） / 16（1608）
 * @param  mode 覆盖方式，显示背景色(0)；叠加方式，不显示背景色(1)
 * 
 * @return
 *     - none
 */
void LCD_ShowFloat(uint16_t x, uint16_t y,uint16_t bcolor, uint16_t fcolor, float num, uint8_t len, uint8_t size, uint8_t mode)
{
	uint8_t i=0;
	char tmp[16]; // 限制最大显示长度为16
	sprintf(tmp, "%.5f", num); // 将浮点数字转换为字符串，限制了最大显示的小数点位数为后5位
	for(i=0;i<len;i++)
	{
		LCD_ShowChar(x+(size/2)*i,y,bcolor,fcolor,tmp[i],size,mode);
	}
}

/**
 * @brief  GUI显示RGB565（16Bit）的图片
 * 		- 一般存储图片使用const，常量数据被放置到DROM中，而DMA无法访问该数据。所以需要先放入行缓存，而不能由const直接发送
 * 
 * @param  x 起点横坐标
 * @param  y 起点纵坐标
 * @param  length 图片长度
 * @param  width 图片宽度
 * @param  p 图片指针
 * 
 * @return
 *     - none
 */
void LCD_ShowPicture_16b(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t *p)
{
	uint16_t i,j;
	LCD_SetWindows(x,y,x+length-1,y+width-1);//窗口设置
	/*// 按点刷，每刷一个点要重复构建SPI-DMA传输，效率低
	for(i=0;i<length;i++)
	{
		for(j=0;j<width;j++)
		{
			picL=*(p+(i*width+j)*2);	//数据低位在前
			picH=*(p+(i*width+j)*2+1);
			LCD_WR_DATA16(picH<<8|picL);
		}
	}*/
	// 按行刷，与 LCD_Clear 共用行缓存。极大减少了构建SPI-DMA的数目，大幅度提高刷屏速度
	for(i=0;i<length;i++)
	{
		for(j=0;j<width;j++)
		{
#if !(CONFIG_LCD_TYPE_ILI9488 || CONFIG_LCD_TYPE_ILI9481)
			// 计算单行像素缓存内容。16Bit/像素，2字节/像素
			lineBuf[j*2] = *(p+(i*width+j)*2+1);//数据低位在前
			lineBuf[j*2+1] = *(p+(i*width+j)*2);
#else
			// ili9488用SPI总线驱动，像素必须为18Bit/pixel，16Bit会没有显示。（需要提前设置0x3A寄存器）
			// 计算单行像素缓存内容。18Bit/像素，3字节/像素
			lineBuf[j*3] = (*(p+(i*width+j)*2+1))&0xF8;//RED
			lineBuf[j*3+1] = (((*(p+(i*width+j)*2+1))&0x07)<<5) | (((*(p+(i*width+j)*2))&0xE0)>>3);//(color>>3)&0xFC;//GREEN
			lineBuf[j*3+2] = (*(p+(i*width+j)*2))<<3;//BLUE
#endif
		}

#if !(CONFIG_LCD_TYPE_ILI9488 || CONFIG_LCD_TYPE_ILI9481)
		lcd_data(LCD_SPI, lineBuf, width*2);
#else
		lcd_data(LCD_SPI, lineBuf, width*3);
#endif
	}
}
