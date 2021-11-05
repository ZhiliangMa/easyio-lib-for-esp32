#include "spi_as5047p.h"

// AS5047P与SPI关联的句柄，通过此来调用SPI总线上的AS5047P设备
spi_device_handle_t AS5047P_SPI = NULL;

/* 从data中获取第n bit的值 注：data只能为uint8*类型指针 */
#define GET_BIT_N_VAL(data, n)  \
(0x1 & (( *((data) + (n)/8) & (0x1 << ((n)%8)) ) >> ((n)%8)))

// 偶校验，来源：https://www.cnblogs.com/mrlayfolk/p/15086822.html
static uint8_t soc_gen_even_parity_common(uint8_t *entry_data, uint16_t entry_len)
{
    uint32_t i = 0;
    uint32_t even_parity = 0;
    for(i = 0; i < entry_len; i++)
    {
        even_parity += GET_BIT_N_VAL((entry_data), i);
    }

    return (even_parity & 0x1);
}

/**
 * @brief  spi总线发送并接收一帧uint16的数据，用以AS5047P通信
 *      - 例：data = as5047p_spi_send_and_recv_uint16(spi, addr, cs_io_num);
 * 
 * @param  spi AS5047P与SPI关联的句柄，通过此来调用SPI总线上的AS5047P设备
 * @param  senddata spi发送的uint16数据
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - spi接收到的uint16数据（重新经过大小端排序后的数据）
 */
static uint16_t as5047p_spi_send_and_recv_uint16(spi_device_handle_t spi, uint16_t senddata, gpio_num_t cs_io_num)
{
    uint8_t temp = 0;
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       // 清空传输结构体

    uint8_t p_data[2];              // 由于ESP32大小端的问题，传输uint16数据需要进行转换调换uint8_t的顺序
    p_data[0] = senddata>>8;
    p_data[1] = senddata;

    // 软件CSn
    gpio_set_level(cs_io_num, 0);

    t.length=8*2;                   // SPI发送AS5047P的CMD长度：16Bit。2个字节。
    t.tx_buffer=p_data;             // 命令是经过偶运算的addr，直接&addr会引起大小端错误
    t.flags = SPI_TRANS_USE_RXDATA;
    //t.user=(void*)0;                // D/C 线电平为0，传输命令
    ret=spi_device_polling_transmit(spi, &t);  // 开始传输
    assert(ret==ESP_OK);            // 应该没有问题

    // 软件CSn
    gpio_set_level(cs_io_num, 1);

    // 将接收到的数据按大小端重新排列
    temp = *t.rx_data;
    *t.rx_data = *(t.rx_data+1);
    *(t.rx_data+1) = temp;

    return *(uint16_t*)t.rx_data; // 返回经过重新大小端排列的数据
}

/**
 * @brief  向AS5047P发送命令帧的同时，同时读返回的数据（读寄存器的值，返回值是AS5047上一次命令返回数据）
 *      - 例：pos = as5047p_cmd_frame_and_read(spi, addr, cs_io_num);
 * 
 * @param  spi AS5047P与SPI关联的句柄，通过此来调用SPI总线上的AS5047P设备
 * @param  addr 读AS5047P寄存器的地址，uint16
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - AS5047P寄存器的值（滤除掉PARD偶校验位，保留原始数据 和 错误标志位。如返回值错误，值会 >= 16384）
 */
static uint16_t as5047p_cmd_frame_and_read(spi_device_handle_t spi, uint16_t addr, gpio_num_t cs_io_num)
{
    uint16_t even = 0;
    uint16_t data, data_e = 0; // 读取的原始数据，和清除掉PARD偶校验位的数据

    // 置读写标志位，置PARC偶校验位
    addr |= 0x4000;                 // 读写标志位，置1，表示读
    even = soc_gen_even_parity_common(&addr, 16); // PARC偶校验
    addr = addr | (even << 15);

    // 开始SPI通信，接收同时读取的数据
    data = as5047p_spi_send_and_recv_uint16(spi, addr, cs_io_num); // 数据帧内的原始数据，有校验位、错误标志位
    data_e = data & 0x7FFF; // 将前1Bit清0，只留下数据和错误标志位，用于校验比对

    // 重新偶校验，如校验错误返回 0xFFFF。 如校验正确，但因为错误标志位置高会导致数据>=16384
    // 调用该读数据的函数，可直接判断是否 >= 16384，来判断是否正确（<为正确，>=为错误）
    even = soc_gen_even_parity_common(&data_e, 16); // PARC偶校验
    // 如果校验错误，或者Bit14为1，则报错，返回值会 >= 16384
    if (even == (data>>15))
        return data_e;
    else
        return 0xFFFF;
}

/**
 * @brief  读AS5047P角度的原始值（适用于只读一次的情况）
 *      - 例：angle = as5047p_read_angle_once(AS5047P_SPI, AS5047P_SOFT_CS0);
 * 
 * @param  spi AS5047P与SPI关联的句柄，通过此来调用SPI总线上的AS5047P设备
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - AS5047P的角度值，0~16383 => 0~360°（如角度返回错误，值会 >= 16384）
 */
uint16_t as5047p_read_angle_once(spi_device_handle_t spi, gpio_num_t cs_io_num)
{
    as5047p_cmd_frame_and_read(spi, ANGLECOM_REGISTER, cs_io_num);

    return as5047p_cmd_frame_and_read(spi, ANGLECOM_REGISTER, cs_io_num);
}

/**
 * @brief  读AS5047P角度的原始值（适用于连续读的情况，需要注意当前的数据是上一帧命令的返回值）
 *      - 例：angle = as5047p_read_angle_continuous(AS5047P_SPI, AS5047P_SOFT_CS0);
 * 
 * @param  spi AS5047P与SPI关联的句柄，通过此来调用SPI总线上的AS5047P设备
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - AS5047P的角度原始值，0~16383 => 0~360°（如角度返回错误，值会 >= 16384）
 */
uint16_t as5047p_read_angle_continuous(spi_device_handle_t spi, gpio_num_t cs_io_num)
{
    return as5047p_cmd_frame_and_read(spi, ANGLECOM_REGISTER, cs_io_num);
}

/**
 * @brief  将AS5047P角度的原始值转换为实际360°角度（如角度返回错误，值会 >= 360°）
 *      - 例：angle = as5047p_to_angle(data);
 * 
 * @param  data 输入读AS5047P角度的原始值
 * 
 * @return
 *     - AS5047P的实际角度值，0~16383 => 0~360°（如角度返回错误，值会 >= 360°）
 */
float as5047p_to_angle(uint16_t data)
{
    float angle = data;
    angle = angle*360/16384;
    return angle;
}

/**
 * @brief  AS5047P初始化，初始化SPI总线，配置为 SPI mode 1.(CPOL=0, CPHA=1)，CS引脚使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 *      - AS5047P的初始化除了设置SPI总线，没有其他过程，不用配置寄存器。电后至少延时等待tpon=10ms。进行SPI通信即可。
 *      - 例：spi_as5047p_init(SPI3_HOST, 100*1000, AS5047P_SOFT_CS0);
 * 
 * @param  host_id SPI端口号。SPI1_HOST / SPI2_HOST / SPI3_HOST
 * @param  clk_speed AS5047P设备的SPI速度（注意：普通GPIO最大只能30MHz，而AS5047P的最快SPI速率为10MHz）
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - none
 */
void spi_as5047p_init(spi_host_device_t host_id, uint32_t clk_speed, gpio_num_t cs_io_num)
{
    esp_err_t ret;

    // AS5047P设备初始化
    // 先关联 SPI总线及AS5047P设备
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=clk_speed,              // CLK时钟频率
        .mode=1,                                // SPI mode 1.(CPOL=0, CPHA=1)
        .spics_io_num=-1,                       // CS引脚定义
        .queue_size=7,                          // 事务队列大小为7
        //.pre_cb=as5047p_spi_pre_transfer_callback,  // 指定预传输回调，以处理 D/C线电平，来区别发送命令/数据
    };
    // 将AS5047P外设与SPI总线关联
    ret=spi_bus_add_device(host_id, &devcfg, &AS5047P_SPI);
    ESP_ERROR_CHECK(ret);

    // 配置软件cs引脚
    gpio_pad_select_gpio(cs_io_num);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(cs_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(cs_io_num, 1);

    // 上电后至少延时等待tpon=10ms。才可以进行SPI通信。
    vTaskDelay(200 / portTICK_PERIOD_MS);
}
