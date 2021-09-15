#include "spi_lcd.h"

// LCD与SPI关联的句柄，通过此来调用SPI总线上的LCD设备
spi_device_handle_t LCD_SPI = NULL;

/**
 * @brief  向LCD发送1个字节的命令（D/C线电平为0）
 *      - 使用spi_device_polling_transmit，它等待直到传输完成。
 *      - 发送时同时设置D/C线为0，传输命令
 *      - 例：lcd_cmd(LCD_SPI, 0x04);
 * 
 * @param  spi LCD与SPI关联的句柄，通过此来调用SPI总线上的LCD设备
 * @param  cmd 向LCD发送的1个字节命令内容
 * 
 * @return
 *     - none
 */
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       // 清空传输结构体
    t.length=8;                     // SPI传输lcd命令的长度：8Bit。1个字节。（lcd的命令都是单字节的）
    t.tx_buffer=&cmd;               // 数据是cmd本身
    t.user=(void*)0;                // D/C 线电平为0，传输命令
    ret=spi_device_polling_transmit(spi, &t);  // 开始传输
    assert(ret==ESP_OK);            // 应该没有问题
}

/**
 * @brief  向LCD发送长度为len个字节的数据（D/C线电平为1）
 *      - 发送时同时设置D/C线为1，传输数据
 *      - 例：lcd_data(LCD_SPI, dataBuf, 10);
 * 
 * @param  spi LCD与SPI关联的句柄，通过此来调用SPI总线上的LCD设备
 * @param  data 要发送数据的指针
 * @param  len 发送的字节数
 * 
 * @return
 *     - none
 */
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             // len为0直接返回，无需发送任何东西
    memset(&t, 0, sizeof(t));       // 清空传输结构体
    t.length=len*8;                 // len单位为字节, 发送长度的单位是Bit
    t.tx_buffer=data;               // 数据指针
    t.user=(void*)1;                // D/C 线电平为1，传输数据
    ret=spi_device_polling_transmit(spi, &t);  // 开始传输
    assert(ret==ESP_OK);            // 应该没有问题
}

/**
 * @brief  向LCD发送单点16Bit的像素数据，（根据驱动IC的不同，可能为2或3个字节，需要转换RGB565、RGB666）
 *      - ili9488\ili9481 这类IC，SPI总线仅能使用RGB666-18Bit/像素，分3字节传输。而不能使用16Bit/像素，分2字节传输。（0x3A寄存器）
 *      - 例：lcd_data16(LCD_SPI, RED);
 * 
 * @param  spi LCD与SPI关联的句柄，通过此来调用SPI总线上的LCD设备
 * @param  data 要发送的单点像素数据，uint16_t的RGB565
 * 
 * @return
 *     - none
 */
void lcd_data16(spi_device_handle_t spi, uint16_t data)
{
#if !(CONFIG_LCD_TYPE_ILI9488 || CONFIG_LCD_TYPE_ILI9481)
    // 16Bit/像素，2字节/像素
    esp_err_t ret;
	spi_transaction_t t;
	uint8_t dataBuf[2] = {0};
	dataBuf[0] = data>>8;
	dataBuf[1] = data&0xFF;
	memset(&t, 0, sizeof(t));		// 清空传输结构体
	t.length=2*8;					// SPI传输数据的长度：16Bit。2个字节
	t.tx_buffer=dataBuf;			// 数据指针
	t.user=(void*)1;				// D/C 线电平为1，传输数据
	ret=spi_device_polling_transmit(spi, &t);   // 开始传输
	assert(ret==ESP_OK);			// 应该没有问题
#else
    // ili9488用SPI总线驱动，像素必须为18Bit/pixel，3字节/像素。16Bit会没有显示。（需要提前设置0x3A寄存器）
    esp_err_t ret;
	spi_transaction_t t;
	uint8_t dataBuf[3] = {0};
    // 将原本2个字节的RGB565，转换为3个字节的RGB666
	dataBuf[0] = (data>>8)&0xF8;//RED
	dataBuf[1] = (data>>3)&0xFC;//GREEN
    dataBuf[2] = data<<3;//BLUE
	memset(&t, 0, sizeof(t));		// 清空传输结构体
	t.length=3*8;					// SPI传输数据的长度：24Bit。3个字节
	t.tx_buffer=dataBuf;			// 数据指针
	t.user=(void*)1;				// D/C 线电平为1，传输数据
	ret=spi_device_polling_transmit(spi, &t);   // 开始传输
	assert(ret==ESP_OK);			// 应该没有问题*/
#endif
}

/**
 * @brief  获取LCD的ID
 *      - 由于通常MISO引脚都不接，而且各驱动IC的寄存器定义有差异，导致大多数情况都得不到LCD的ID
 *      - 例：lcd_get_id(LCD_SPI);
 * 
 * @param  spi LCD与SPI关联的句柄，通过此来调用SPI总线上的LCD设备
 * 
 * @return
 *     - none
 */
static uint32_t lcd_get_id(spi_device_handle_t spi)
{
    //get_id cmd
    lcd_cmd(spi, 0x04);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=8*3;
    t.flags = SPI_TRANS_USE_RXDATA;
    t.user = (void*)1;

    esp_err_t ret = spi_device_polling_transmit(spi, &t);
    assert( ret == ESP_OK );

    return *(uint32_t*)t.rx_data;
}

/**
 * @brief  初始化LCD的驱动IC
 *      - 根据 spi_lcd.h 中对IC型号的定义，将预先准备好的参数写入到LCD驱动IC内
 *      - lcd_ic_init(LCD_SPI);
 * 
 * @param  spi LCD与SPI关联的句柄，通过此来调用SPI总线上的LCD设备
 * 
 * @return
 *     - none
 */
static void lcd_ic_init(spi_device_handle_t spi)
{
    int cmd=0;
    const lcd_init_cmd_t* lcd_init_cmds;

    // 初始化 DC、RST、BLK 引脚，配置为推挽输出
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    // RST引脚拉低，复位后重新拉高
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);

    /*// 检测LCD的驱动IC型号，以作驱动适配
    uint32_t lcd_id = lcd_get_id(spi);
    int lcd_detected_type = 0;
    int lcd_type;

    printf("LCD ID: %08X\n", lcd_id);
    if ( lcd_id == 0 ) {
        //zero, ili
        lcd_detected_type = LCD_TYPE_ILI_9341;
        printf("ILI9341 detected.\n");
    } else {
        // none-zero, ST
        lcd_detected_type = LCD_TYPE_ST_7789V;
        printf("ST7789V detected.\n");
    }*/

    int lcd_detected_type = 0;
    int lcd_type;

#ifdef CONFIG_LCD_TYPE_AUTO
    lcd_type = lcd_detected_type;
#elif defined( CONFIG_LCD_TYPE_ST7735 )
    printf("LCD ST7735 initialization.\n");
    lcd_init_cmds = st_7735_init_cmds; // st_init_cmds;
    lcd_type = LCD_TYPE_ST_7735;
#elif defined( CONFIG_LCD_TYPE_ST7735S )
    printf("LCD ST7735 initialization.\n");
    lcd_init_cmds = st_7735S_init_cmds; // st_init_cmds;
    lcd_type = LCD_TYPE_ST_7735S;
#elif defined( CONFIG_LCD_TYPE_ST7789V )
    printf("LCD ST7789V initialization.\n");
    lcd_init_cmds = st_7789V_init_cmds;
    lcd_type = LCD_TYPE_ST_7789V;
#elif defined( CONFIG_LCD_TYPE_ILI9341 )
    printf("LCD ILI9341 initialization.\n");
    lcd_init_cmds = ili_9341_init_cmds;
    lcd_type = LCD_TYPE_ILI_9341;
#elif defined( CONFIG_LCD_TYPE_ILI9488 )
    printf("LCD ILI9488 initialization.\n");
    lcd_init_cmds = ili_9488_init_cmds;
    lcd_type = LCD_TYPE_ILI_9488;
#elif defined( CONFIG_LCD_TYPE_ILI9481 )
    printf("LCD ILI9481 initialization.\n");
    lcd_init_cmds = ili_9481_init_cmds;
    lcd_type = LCD_TYPE_ILI_9481;
#elif defined( CONFIG_LCD_TYPE_HX8357C )
    printf("LCD HX8357C initialization.\n");
    lcd_init_cmds = hx_8357c_init_cmds;
    lcd_type = LCD_TYPE_HX_8357C;
#endif

    // 将spi_lcd.h 中的参数写入LCD驱动IC
    while (lcd_init_cmds[cmd].databytes!=0xff) {
        lcd_cmd(spi, lcd_init_cmds[cmd].cmd);
        lcd_data(spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes&0x1F);
        if (lcd_init_cmds[cmd].databytes&0x80) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
        cmd++;
    }
}

// SPI预传输回调。（在SPI传输即将开始前调用的函数）用来处理 D/C 线的电平，来控制接下来发送的内容是 指令/数据
static void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

/**
 * @brief  以SPI方式驱动LCD初始化函数
 *      - 过程包括：关联 SPI总线及LCD设备、驱动IC的参数配置、点亮背光、设置LCD的安装方向、设置屏幕分辨率、扫描方向、初始化显示区域的大小
 *      - （注意：普通GPIO最大只能30MHz，而IOMUX默认的SPI，CLK最大可以设置到80MHz）
 *      - 例：spi_lcd_init(SPI2_HOST, 60*1000*1000, LCD_SPI2_DEF_PIN_NUM_CS0);
 * 
 * @param  host_id SPI端口号。SPI1_HOST / SPI2_HOST / SPI3_HOST
 * @param  clk_speed LCD设备的SPI速度（注意：普通GPIO最大只能30MHz，而IOMUX默认的SPI，CLK最大可以设置到80MHz）
 * @param  cs_io_num CS端口号，尽量使用IOMUX默认的IO
 * 
 * @return
 *     - none
 */
void spi_lcd_init(spi_host_device_t host_id, uint32_t clk_speed, gpio_num_t cs_io_num)
{
    esp_err_t ret;

    // LCD设备初始化
    // 先关联 SPI总线及LCD设备
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=clk_speed,              // CLK时钟频率
        .mode=0,                                // SPI mode 0
        .spics_io_num=cs_io_num,                // CS引脚定义
        .queue_size=7,                          // 事务队列大小为7
        .pre_cb=lcd_spi_pre_transfer_callback,  // 指定预传输回调，以处理 D/C线电平，来区别发送命令/数据
    };
    // 将LCD外设与SPI总线关联
    ret=spi_bus_add_device(host_id, &devcfg, &LCD_SPI);
    ESP_ERROR_CHECK(ret);

    // LCD 驱动IC的参数配置（自动识别始终不成功、需要到.h文件中手动配置）
    lcd_ic_init(LCD_SPI);

    // 点亮背光
    gpio_set_level(PIN_NUM_BCKL, 1);

    // 设置LCD的安装方向、及横竖方向的像素数目（需要与下面的扫描方式保持一致）
	LCD_Display_Resolution(horizontal);

    // 设置屏幕分辨率、扫描方向
    // 初始化 显示区域的大小，和扫描方向。（！！重要，必有，否则不能显示正常）
    // 来匹配屏幕的安装方向。或镜像安装方式（可用于镜面反射及棱镜的镜像显示）（提供了8中扫描方式，以便横竖屏、翻转和镜像的切换）
    LCD_Display_Dir(LCD_DIR, LCD_INVERT, LCD_MIRROR);
    //LCD_Display_Dir(vertical, invert_dis, mirror_dis); // 竖屏、不倒置(正着摆放)、不镜像
    //LCD_Display_Dir(vertical, invert_dis, mirror_en); // 竖屏、不倒置(正着摆放)、镜像
    //LCD_Display_Dir(vertical, invert_en, mirror_dis); // 竖屏、倒置(倒立摆放)、不镜像
    //LCD_Display_Dir(vertical, invert_en, mirror_en); // 竖屏、倒置(倒立摆放)、镜像
    //LCD_Display_Dir(horizontal, invert_dis, mirror_dis); // 横屏、不倒置(正着摆放)、不镜像
    //LCD_Display_Dir(horizontal, invert_dis, mirror_en); // 横屏、不倒置(正着摆放)、镜像
    //LCD_Display_Dir(horizontal, invert_en, mirror_dis); // 横屏、倒置(倒立摆放)、不镜像
    //LCD_Display_Dir(horizontal, invert_en, mirror_en); // 横屏、倒置(倒立摆放)、镜像
}

// 推荐一个很全的LCD驱动库，arduino 的TFT_eSPI库：https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_Drivers/ILI9341_Init.h
// 不过easyIO的代码并没有参照这个，因为还是要以买到的LCD的卖家资料为首要参考，有时相同IC但不同厂家的LCD的驱动参数都会不同。
