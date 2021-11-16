#include "lcd_spi_dma.h"

static const char *TAG = "lcd_dma";

// *lines[2]、sending_line、calc_line 不能被其他函数调用及修改，会造成LCD-SPI-DMA刷屏崩溃
// 因为LCD的帧显示 使用相同的缓冲区，和标志位，当前帧发送不能对下一次发送标志位和缓存造成改动。
// 为了防止本次未完成的发送对下一帧数据造成影响，末尾要等待上一次的发送完成、
// 修改后，会因为遗漏 send_line_finish(spi); 造成在下一次调用 spi_device_queue_trans 时引起系统崩溃


/**
 * @brief  初始化lcd使用`SPI-DMA 双缓冲环形队列`刷屏（申请DMA双缓冲内存）
 *      - 会根据"spi_lcd.h"中定义的屏幕尺寸，自动匹配行像素的值，并根据行像素大小申请内存，所以不需要传入参数）
 * 
 * @param  void
 * 
 * @return
 *     - void
 */
// SPI-DMA双缓冲环形队列的内存区指针
uint16_t *lines[2];
// 当前发送到LCD-SPI的行和正在计算的行的索引值。(display_img也会用得到，全局变量)
int sending_line=-1;
int calc_line=0;
void lcd_spi_dma_display_init(void)
{
    // 为像素缓冲区分配内存，2个形成DMA环形发送队列（分配内存大小与 行像素个数-如320，和 PARALLEL_LINES 有关）
    for (int i=0; i<2; i++) {
        lines[i]=heap_caps_malloc(PARALLEL_LINES*lcddev.width*sizeof(uint16_t), MALLOC_CAP_DMA);
        assert(lines[i]!=NULL);
    }
}

// 将解码出的图片RGB565缓存，搬运到LCD-DMA发送缓冲
// SPI-DMA整页刷新，使用的是环形DMA，需要有两个缓冲区，一个在发送时不能动，另一个计算用于准备下一次发送。
// 并且由于使用DMA队列，发送时几乎不占用CPU，而这部分空闲时间正好拿来搬运和处理下一行的像素数据，两边各不耽误
static inline uint16_t get_bgnd_pixel(uint16_t **pixels, int x, int y)
{
    return pixels[y][x];
}
static void load_img_line(uint16_t *pixels, uint16_t *dest, int line, int linect)
{
    for (int y=line; y<line+linect; y++) {
        for (int x=0; x<320; x++) {
            *dest++=get_bgnd_pixel(pixels, x, y);
        }
    }
}

static void send_line_finish(spi_device_handle_t spi)
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
    //Wait for all 6 transactions to be done and get back the results.
    for (int x=0; x<6; x++) {
        ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
        assert(ret==ESP_OK);
        //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
    }
}

/* 发送一组行像素，必须先发送一个命令、2个数据字、另一个命令、另外2个数据字和另一个命令，然后再发送行数据本身
 * （0x2A+4字节数据+0x2B+4字节数据+0x2c+像素数据）
 * 共6笔事务。 （我们不能将所有这些都放在一个事务中，因为D / C行需要在中间切换。）
 * 该例程将这些命令作为中断事务排入队列，以便它们发送得更快（与多次调用spi_device_transmit相比），同时可以计算出下一个事务的行数。
 */
static void send_lines(spi_device_handle_t spi, int ypos, uint16_t *linedata)
{
    esp_err_t ret;
    int x;
    // 传输描述符，声明为静态，因此它们不分配在堆栈上; 
    // 即使此函数已运行完成，我们也需要此内存。 因为SPI驱动程序仍需要访问它，即使我们已经在计算下一行，
    // 该结构描述了一个SPI事务。 事务完成之前，不应修改描述符。
    static spi_transaction_t trans[6];

    //In theory, it's better to initialize trans and data only once and hang on to the initialized
    //variables. We allocate them on the stack, so we need to re-init them each call.
    for (x=0; x<6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x&1)==0) {
            //Even transfers are commands
            trans[x].length=8;
            trans[x].user=(void*)0;
        } else {
            //Odd transfers are data
            trans[x].length=8*4;
            trans[x].user=(void*)1;
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0]=0x2A;           //Column Address Set
    trans[1].tx_data[0]=0;              //Start Col High
    trans[1].tx_data[1]=0;              //Start Col Low
    trans[1].tx_data[2]=(lcddev.width)>>8;       //End Col High
    trans[1].tx_data[3]=(lcddev.width)&0xff;     //End Col Low
    trans[2].tx_data[0]=0x2B;           //Page address set
    trans[3].tx_data[0]=ypos>>8;        //Start page high
    trans[3].tx_data[1]=ypos&0xff;      //start page low
    trans[3].tx_data[2]=(ypos+PARALLEL_LINES)>>8;    //end page high
    trans[3].tx_data[3]=(ypos+PARALLEL_LINES)&0xff;  //end page low
    trans[4].tx_data[0]=0x2C;           //memory write
    trans[5].tx_buffer=linedata;        //finally send the line data
    trans[5].length=PARALLEL_LINES*lcddev.width*2*8;          //Data length, in bits。一次传输PARALLEL_LINES行
    trans[5].flags=0; //undo SPI_TRANS_USE_TXDATA flag

    // 队列传输
    for (x=0; x<6; x++) {
        ret=spi_device_queue_trans(spi, &trans[x], portMAX_DELAY);
        assert(ret==ESP_OK);
    }

    // 当运行到这里，SPI驱动程序正忙于发送事务（在后台）
    // 主要是使用DMA发送的，因此CPU在这里没有太多工作要做。
    // 我们不会等待交易完成，因为我们要花时间去计算下一行
    // 完成后，我们可以调用send_line_finish，它将检查其状态并等待传输完成。
}

/**
 * @brief  刷一整个LCD屏幕（传输使用DMA环形队列加速）
 *      - DMA双缓冲，可以充分利用CPU资源；分行传输，可以减少RAM占用
 * 
 * @param  spi SPI总线与LCD设备关联的句柄
 * @param  pixels 用于刷新屏幕GRAM的RGB565像素内容的指针
 * 
 * @return
 *     - void
 */
void lcd_spi_dma_display_img(spi_device_handle_t spi, uint16_t ***pixels)
{
    for (int y=0; y<lcddev.height; y+=PARALLEL_LINES) {
        // 加载一行像素内容（实际是 PARALLEL_LINES 行，每行用队列依次发送）
        load_img_line(*pixels, lines[calc_line], y, PARALLEL_LINES);
        // 完成上一行的发送过程
        if (sending_line!=-1) send_line_finish(spi);
        // 交换sending_line和calc_line
        sending_line=calc_line;
        calc_line=(calc_line==1)?0:1;
        // 发送刚刚加载过的行像素
        send_lines(spi, y, lines[sending_line]);
        // 该行像素已排队等待立即发送； 实际的发送发生在后台
        // 只要不动line [sending_line]，我们就可以继续计算下一行。 SPI发送过程仍在进行中。
    }
}




// get_bgnd_pixel 和 get_bgnd_pixel_offset不一样，后者有像素偏移
// 如果pretty_effect_calc_lines内不小心使用了get_bgnd_pixel，会因为指针过界，导致运行反复重启
static inline uint16_t get_bgnd_pixel_offset(uint16_t **pixels, int x, int y)
{
    // 波动动效的图片具有8x8像素边距，需要带入偏移量
    x+=8;
    y+=8;
    return pixels[y][x];
}

// 此变量用于检测下一帧。
static int prev_frame=-1;
// 每当帧发生变化时，我们都会预先计算值，而不是为我们抓取的每个像素计算偏移量，
// 然后在遍历帧中的所有像素时重新使用这些像素。 这快得多了。
static int8_t xofs[320], yofs[240];
static int8_t xcomp[320], ycomp[240];

// 计算一组线的像素数据（每条线大小为320），输入的静态图像输出动态波动图
// dest 是输入的像素数据。line是要计算的第一行的Y坐标。linect是要计算的行数
// 每次显示整个图像时，帧会增加一格； 这用于转到动画的下一帧。
static void pretty_effect_calc_lines(uint16_t *pixels, uint16_t *dest, int line, int frame, int linect)
{
    if (frame!=prev_frame) {
        // 我们需要计算一组新的偏移系数。 以一些随机的正弦作为偏移量，以使所有内容看起来既漂亮又流畅。
        for (int x=0; x<320; x++) xofs[x]=sin(frame*0.15+x*0.06)*4;
        for (int y=0; y<240; y++) yofs[y]=sin(frame*0.1+y*0.05)*4;
        for (int x=0; x<320; x++) xcomp[x]=sin(frame*0.11+x*0.12)*4;
        for (int y=0; y<240; y++) ycomp[y]=sin(frame*0.07+y*0.15)*4;
        prev_frame=frame;
    }
    // 搬运源图片数据，过来进行动效处理
    for (int y=line; y<line+linect; y++) {
        for (int x=0; x<320; x++) {
            *dest++=get_bgnd_pixel_offset(pixels, x+yofs[y]+xcomp[x], y+xofs[x]+ycomp[y]);
        }
    }
}

/**
 * @brief  使用波动动效刷一整个LCD屏幕（传输使用DMA环形队列）并在lcd右下角显示帧率
 *      - 在lcd_spi_dma_display_img的基础上，添加了波动动效
 * 
 * @param  spi SPI总线与LCD设备关联的句柄
 * @param  pixels 用于刷新屏幕GRAM的RGB565像素内容的指针
 * 
 * @return
 *     - void
 */
void lcd_spi_dma_display_pretty_colors(spi_device_handle_t spi, uint16_t ***pixels)
{
    int frame=0;
    int64_t t_fps, t_fps_last=0;

    while(1) {
        frame++;
        for (int y=0; y<lcddev.height; y+=PARALLEL_LINES) {
            // 计算并加载一行像素内容（实际是 PARALLEL_LINES 行，每行用队列依次发送）
            pretty_effect_calc_lines(*pixels, lines[calc_line], y, frame, PARALLEL_LINES);
            // 完成上一行的发送过程
            if (sending_line!=-1) send_line_finish(spi);
            // 交换sending_line和calc_line
            sending_line=calc_line;
            calc_line=(calc_line==1)?0:1;
            // 发送刚刚计算的行。
            send_lines(spi, y, lines[sending_line]);
            // 该行像素已排队等待立即发送； 实际的发送发生在后台
            // 只要不动line [sending_line]，我们就可以继续计算下一行。 SPI发送过程仍在进行中。
        }

        // 计算lcd用SPI-DMA 双缓冲环形队列刷屏的帧率 (用逐帧首尾计时的方式，不会因LCD显示调试信息所耗费的时间，而影响到显示的帧率)
        t_fps = esp_timer_get_time(); //获取当前系统时间。计数1000是1ms，1s是1000000
        ESP_LOGI(TAG, "FPS:%.2f\n", 1000000.0f/(t_fps - t_fps_last));
        LCD_ShowString(248-1, 224-1, YELLOW, BLUE, "FPS:", 16, 0);
        LCD_ShowFloat(248+8*4-1, 224-1, YELLOW, BLUE, 1000000.0f/(t_fps - t_fps_last), 5, 16, 0);
        t_fps_last = esp_timer_get_time();
    }
}
