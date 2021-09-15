#include "spi_tle5012b.h"

// TLE5012B与SPI关联的句柄，通过此来调用SPI总线上的TLE5012B设备
spi_device_handle_t TLE5012B_SPI = NULL;

/**
 * @brief  spi总线发送并接收一帧uint16的数据，用以TLE5012B通信
 *      - 例：data = tle5012b_spi_send_and_recv_uint16(spi, 0x0000, cs_io_num);
 * 
 * @param  spi TLE5012B与SPI关联的句柄，通过此来调用SPI总线上的TLE5012B设备
 * @param  senddata spi发送的uint16数据
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致TLE5012B通信不正常）
 * 
 * @return
 *     - spi接收到的uint16数据（重新经过大小端排序后的数据）
 */
static uint16_t tle5012b_spi_send_and_recv_uint16(spi_device_handle_t spi, uint16_t senddata, gpio_num_t cs_io_num)
{
    uint8_t temp = 0;
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       // 清空传输结构体

    uint8_t p_data[2];              // 由于ESP32大小端的问题，传输uint16数据需要进行转换调换uint8_t的顺序
    p_data[0] = senddata>>8;
    p_data[1] = senddata;

    t.length=8*2;                   // SPI发送TLE5012B的CMD长度：16Bit。2个字节。
    t.tx_buffer=p_data;             // 命令是经过偶运算的addr，直接&addr会引起大小端错误
    t.flags = SPI_TRANS_USE_RXDATA;
    //t.user=(void*)0;                // D/C 线电平为0，传输命令
    ret=spi_device_polling_transmit(spi, &t);  // 开始传输
    assert(ret==ESP_OK);            // 应该没有问题

    // 将接收到的数据按大小端重新排列
    temp = *t.rx_data;
    *t.rx_data = *(t.rx_data+1);
    *(t.rx_data+1) = temp;

    return *(uint16_t*)t.rx_data; // 返回经过重新大小端排列的数据
}

/**
 * @brief  spi总线发送并接收一帧uint32的数据，用以TLE5012B通信
 *      - 例：data = tle5012b_spi_send_and_recv_uint32(spi, 0x0000, cs_io_num);
 * 
 * @param  spi TLE5012B与SPI关联的句柄，通过此来调用SPI总线上的TLE5012B设备
 * @param  senddata spi发送的uint32数据
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致TLE5012B通信不正常）
 * 
 * @return
 *     - spi接收到的uint32数据（重新经过大小端排序后的数据）
 */
static uint32_t tle5012b_spi_send_and_recv_uint32(spi_device_handle_t spi, uint32_t senddata, gpio_num_t cs_io_num)
{
    uint8_t temp = 0;
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       // 清空传输结构体

    uint8_t p_data[4];              // 由于ESP32大小端的问题，传输uint32数据需要进行转换调换uint8_t的顺序
    p_data[0] = senddata>>24;
    p_data[1] = senddata>>16;
    p_data[2] = senddata>>8;
    p_data[3] = senddata;

    t.length=8*4;                   // SPI发送TLE5012B长度：32Bit。4个字节。
    t.tx_buffer=p_data;             // 命令是经过偶运算的addr，直接&addr会引起大小端错误
    t.flags = SPI_TRANS_USE_RXDATA;
    //t.user=(void*)0;                // D/C 线电平为0，传输命令
    ret=spi_device_polling_transmit(spi, &t);  // 开始传输
    assert(ret==ESP_OK);            // 应该没有问题

    // 将接收到的数据按大小端重新排列
    temp = *t.rx_data;
    *t.rx_data = *(t.rx_data+3);
    *(t.rx_data+3) = temp;

    temp = *(t.rx_data+1);
    *(t.rx_data+1) = *(t.rx_data+2);
    *(t.rx_data+2) = temp;

    return *(uint32_t*)t.rx_data; // 返回经过重新大小端排列的数据
}

/**
 * @brief  读TLE5012B一个寄存器的值，返回uint16，没有安全字（适用于只读一次的情况）
 *      - 例：tle5012b_read_register_once(spi, CMD_READ_ANGLE, cs_io_num);
 * 
 * @param  spi TLE5012B与SPI关联的句柄，通过此来调用SPI总线上的TLE5012B设备
 * @param  addr 读TLE5012B寄存器的地址，uint16
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - TLE5012B的寄存器的值（清除掉Bit15的1）
 */
static uint16_t tle5012b_read_register_once(spi_device_handle_t spi, uint16_t addr, gpio_num_t cs_io_num)
{
    uint16_t data;

    // 软件CSn
    gpio_set_level(cs_io_num, 0);
    // 发送读命令和寄存地址
    tle5012b_spi_send_and_recv_uint16(spi, addr, cs_io_num); // 数据帧内的原始数据
    // 开始SPI通信，接收同时读取的数据
    data = tle5012b_spi_send_and_recv_uint16(spi, 0x0000, cs_io_num); // 数据帧内的原始数据
    // 软件CSn
    gpio_set_level(cs_io_num, 1);

    data &= 0x7FFF; // 清除掉Bit15的1

    return data;
}

/**
 * @brief  读TLE5012B一个寄存器的值，返回uint16，有安全字判断（适用于只读一次的情况）
 *      - 例：tle5012b_read_register_once_safety(spi, CMD_READ_ANGLE, cs_io_num);
 * 
 * @param  spi TLE5012B与SPI关联的句柄，通过此来调用SPI总线上的TLE5012B设备
 * @param  addr 读TLE5012B寄存器的地址，uint16
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - TLE5012B的寄存器的值（清除掉Bit15的1）
 */
static uint16_t tle5012b_read_register_once_safety(spi_device_handle_t spi, uint16_t addr, gpio_num_t cs_io_num)
{
    uint32_t recv;
    uint16_t data, safety;

    // 软件CSn
    gpio_set_level(cs_io_num, 0);
    // 发送读命令和寄存地址
    tle5012b_spi_send_and_recv_uint16(spi, addr, cs_io_num); // 数据帧内的原始数据
    // 读传感器数据 和 安全字
    recv = tle5012b_spi_send_and_recv_uint32(spi, 0x00000000, cs_io_num); // 数据帧内的原始数据
    // 软件CSn
    gpio_set_level(cs_io_num, 1);

    // 得到寄存器数据并清除掉Bit15的1
    data = recv >> 16;
    data &= 0x7FFF; // 清除掉Bit15的1
    // 得到安全字，暂未处理
    safety = recv;

    return data;
}

/**
 * @brief  读TLE5012B角度的原始值（适用于只读一次的情况）
 *      - 例：xdata = tle5012b_read_angle(TLE5012B_SPI, TLE5012B_SOFT_CS0);
 * 
 * @param  spi TLE5012B与SPI关联的句柄，通过此来调用SPI总线上的TLE5012B设备
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - TLE5012B的角度值原始值，0~32767 => 0~360°.
 */
uint16_t tle5012b_read_angle(spi_device_handle_t spi, gpio_num_t cs_io_num)
{
    return tle5012b_read_register_once(spi, CMD_READ_ANGLE, cs_io_num);
}

/**
 * @brief  读TLE5012B速度的原始值（适用于只读一次的情况）
 *      - 例：xdata = tle5012b_read_speed(TLE5012B_SPI, TLE5012B_SOFT_CS0);
 * 
 * @param  spi TLE5012B与SPI关联的句柄，通过此来调用SPI总线上的TLE5012B设备
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 * 
 * @return
 *     - TLE5012B的速度值原始值。
 */
uint16_t tle5012b_read_speed(spi_device_handle_t spi, gpio_num_t cs_io_num)
{
    return tle5012b_read_register_once(spi, CMD_READ_SPEED, cs_io_num);
}

/**
 * @brief  将TLE5012B角度的原始值转换为实际360°角度
 *      - 例：angle = tle5012_to_angle(data);
 * 
 * @param  data 输入读TLE5012B角度的原始值
 * 
 * @return
 *     - TLE5012B的实际角度值，0~32767 => 0~360°.
 */
float tle5012_to_angle(uint16_t data)
{
    float angle = data;
    angle = angle*360/32768;
    return angle;
}

/**
 * @brief  TLE5012B初始化，初始化SPI总线，配置为 SPI mode 1.(CPOL=0, CPHA=1)，CS引脚使用软件控制（ESP32的硬件CS流控会导致AS5047P通信不正常）
 *      - TLE5012B的初始化除了设置SPI总线，没有其他过程，不用配置寄存器。电后至少延时等待tpon=10ms。进行SPI通信即可。
 *      - 例：spi_tle5012b_init(SPI3_HOST, 4*1000*1000, TLE5012B_SOFT_CS0);
 * 
 * @param  host_id SPI端口号。SPI1_HOST / SPI2_HOST / SPI3_HOST
 * @param  clk_speed TLE5012B设备的SPI速度（注意：普通GPIO最大只能30MHz，而TLE5012B的最快SPI速率为8MHz）
 * @param  cs_io_num CS端口号，使用软件控制（ESP32的硬件CS流控会导致TLE5012B通信不正常）
 * 
 * @return
 *     - none
 */
void spi_tle5012b_init(spi_host_device_t host_id, uint32_t clk_speed, gpio_num_t cs_io_num)
{
    esp_err_t ret;

    // TLE5012B设备初始化
    // 先关联 SPI总线及TLE5012B设备
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=clk_speed,              // CLK时钟频率
        .mode=1,                                // SPI mode 1.(CPOL=0, CPHA=1)
        .spics_io_num=-1,                       // CS引脚定义
        .queue_size=7,                          // 事务队列大小为7
        //.pre_cb=tle5012b_spi_pre_transfer_callback,  // 指定预传输回调，以处理 D/C线电平，来区别发送命令/数据
    };
    // 将TLE5012B外设与SPI总线关联
    ret=spi_bus_add_device(host_id, &devcfg, &TLE5012B_SPI);
    ESP_ERROR_CHECK(ret);

    // 配置软件cs引脚
    gpio_pad_select_gpio(cs_io_num);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(cs_io_num, GPIO_MODE_OUTPUT);
    gpio_set_level(cs_io_num, 1);

    // 上电后至少延时等待tpon=10ms。才可以进行SPI通信。
    vTaskDelay(200 / portTICK_PERIOD_MS);
}
