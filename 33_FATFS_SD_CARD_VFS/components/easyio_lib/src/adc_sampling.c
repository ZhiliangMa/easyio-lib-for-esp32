#include "adc_sampling.h"

static esp_adc_cal_characteristics_t *adc_chars;


//检查 TP两点校准值、Vref参考电压值 是否被刻录到eFuse中
static void check_efuse(void)
{
    //检查 TP两点校准值 是否被刻录到eFuse中（TP两点校准值是用户自己测量，并刻录到eFuse中）
    esp_err_t eFuse_TP = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP);
    //检查 Vref参考电压值 是否被刻录到eFuse中（eFuse Vref由工厂生产时刻录，一般默认用这种方式校准）
    esp_err_t eFuse_Vref = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF);
#if adc_Dlog
    if (eFuse_TP == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    if (eFuse_Vref == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
#endif
    eFuse_TP = eFuse_Vref; //无用，防止编译报警告
    eFuse_Vref = eFuse_TP; //无用，防止编译报警告
}

//打印校准类型
static void print_char_val_type(esp_adc_cal_value_t val_type)
{
#if adc_Dlog
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
#endif
}

/*  ESP32 ADC1 channel 与 Pin 对照表

    +----------+------------+-------+-----------------+--------------+
    |          | channel    |  Pin  |      Name       |   Function   |
    +==========+============+=======+=================+==============+
    |          |  ADC1_CH0  |   5   |    SENSOR_VP    |    GPIO36    |
    |          +------------+-------+-----------------+--------------+
    |          |  ADC1_CH1  |   6   |    SENSOR_CAPP  |    GPIO37    |
    |          +------------+-------+-----------------+--------------+
    |          |  ADC1_CH2  |   7   |    SENSOR_CAPN  |    GPIO38    |
    |          +------------+-------+-----------------+--------------+
    |   ESP32  |  ADC1_CH3  |   8   |    SENSOR_VN    |    GPIO39    |
    |   SAR    +------------+-------+-----------------+--------------+
    |   ADC1   |  ADC1_CH4  |   12  |    32K_XP       |    GPIO32    |
    |          +------------+-------+-----------------+--------------+
    |          |  ADC1_CH5  |   13  |    32K_XN       |    GPIO33    |
    |          +------------+-------+-----------------+--------------+
    |          |  ADC1_CH6  |   10  |    VDET_1       |    GPIO34    |
    |          +------------+-------+-----------------+--------------+
    |          |  ADC1_CH7  |   11  |    VDET_2       |    GPIO35    |
    +----------+------------+-------+-----------------+--------------+

*/

/*  ESP32 ADC 衰减系数 与 量程 对照表

    +----------+------------+--------------------------+------------------+
    |   SoC    | attenuation|   suggested range (mV)   |  full range (V)  |
    +==========+============+==========================+==================+
    |          |  0         |     100 ~ 950            |     0 ~ 1.1      |
    |          +------------+--------------------------+------------------+
    |          |  2.5       |     100 ~ 1250           |     0 ~ 1.5      |
    |   ESP32  +------------+--------------------------+------------------+
    |          |  6         |     150 ~ 1750           |     0 ~ 2.2      |
    |          +------------+--------------------------+------------------+
    |          |  11        |     150 ~ 2450           |     0 ~ 3.9      |
    +----------+------------+--------------------------+------------------+
    |          |  0         |     100 ~ 800            |     0 ~ 1.1      |
    |          +------------+--------------------------+------------------+
    |          |  2.5       |     100 ~ 1100           |     0 ~ 1.5      |
    | ESP32-S2 +------------+--------------------------+------------------+
    |          |  6         |     150 ~ 1350           |     0 ~ 2.2      |
    |          +------------+--------------------------+------------------+
    |          |  11        |     150 ~ 2600           |     0 ~ 3.9      |
    +----------+------------+--------------------------+------------------+

*/

/**
 * @brief  ADC1及输入通道初始化（在特定衰减下表征ADC的特性，并生成ADC电压曲线）
 *      - 支持函数重载，支持输入不定数目的通道参数，ESP32-ADC1通道数目最大值为8，总参数数目为 3~10。
 *        例：
 *          adc1_init_with_calibrate(ADC_ATTEN_DB_0, 1, ADC_CHANNEL_6);
 *          adc1_init_with_calibrate(ADC_ATTEN_DB_11, 1, ADC_CHANNEL_6);
 *          adc1_init_with_calibrate(ADC_ATTEN_DB_11, 2, ADC_CHANNEL_6, ADC_CHANNEL_7);
 *      - 由于ADC2不能与WIFI共用，所以尽量优先使用ADC1，且ADC2的读取方式与ADC1不同，也就没有在esayIO中提供ADC2的初始化和读取函数
 *      - 默认设置ADC转换数据宽度为12Bit，0~4095。（几乎不需要更改，需要更改的情况一般是改为8Bit来缩小RAM占用。但ESP32的ADC不支持8Bit，最低9Bit）
 * 
 * @param  atten    输入通道衰减系数（决定了输入电压量程）。有 0、2.5、6、11DB 可选，详见上表
 * @param  ch_num   总ADC1输入通道 的数量。channel 与 Pin 对照关系，详见上表
 * @param  (...)    ADC1输入通道列表。支持不定数目参数，数目为 1~8。值为 ADC_CHANNEL_0, ADC_CHANNEL_1... ADC_CHANNEL_7。
 * 
 * @return
 *      - none
 * 
 */
void adc1_init_with_calibrate(adc_atten_t atten, int ch_num, ...)
{
    adc_bits_width_t width = ADC_WIDTH_BIT_12; //ADC转换数据宽度为12Bit
    //检查 TP两点校准值、Vref，是否被刻录到eFuse中（一般出厂后eFuse中仅有Vref）
    check_efuse();
    //Configure ADC1
    adc1_config_width(width);

    //由输入的不定数目参数，解析出端口值
    int channels[8] = {0}; //解析出的端口列表
    va_list va;
    va_start(va, ch_num); // init va, pointing to the first argument
    for(int i=0; i<ch_num; i++) {
        channels[i] = va_arg(va,int); // get the next argument, the type is int
    }
    va_end(va);
    //配置 ADC1的端口和衰减
    for(int i=0; i<ch_num; i++) {
        //配置 ADC1的端口和衰减
        adc1_config_channel_atten(channels[i], atten);
    }

    //Characterize ADC
    //申请一段内存，用于存储 esp_adc_cal_characteristics_t 校准参数结构体
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    //在特定衰减下表征ADC的特性，并生成ADC电压曲线，之后调用esp_adc_cal_raw_to_voltage可对转换值进行校准补偿。并返回选用的校准方式：eFuse TP两点、eFuse Vref或 default referenc
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF, adc_chars);
    //打印最终使用的校准类型
    print_char_val_type(val_type);
}

//获取ADC1通道x转换后的原始值
//adc1_get_raw(ADC_CHANNEL_6);

/**
 * @brief  获取ADC1通道x经校准补偿后的转换电压，单位mV
 *      - 注意：使用该函数前，一定要用 adc1_init_with_calibrate 对ADC1进行校准并初始化，否则会报错
 * 
 * @param  channel 要读取的ADC1输入通道
 * 
 * @return
 *     - 电压值，单位mV。uint32_t类型
 * 
 */
uint32_t adc1_cal_get_voltage(adc_channel_t channel)
{
    uint32_t adc_reading = adc1_get_raw(channel);
    //Convert adc_reading to voltage in mV
    return esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
}

/**
 * @brief  获取ADC1通道x，经多重采样平均后，并校准补偿后的转换电压，单位mV
 *      - 注意：使用该函数前，一定要用 adc1_init_with_calibrate 对ADC1进行校准并初始化，否则会报错
 * 
 * @param  channel 要读取的ADC1输入通道
 * @param  mul_num 多重采样的次数
 * 
 * @return
 *     - 电压值，单位mV。uint32_t类型
 * 
 */
uint32_t adc1_cal_get_voltage_mul(adc_channel_t channel, uint32_t mul_num)
{
    uint32_t adc_reading = 0;
    for(uint32_t i=0; i<mul_num; i++) {
        adc_reading += adc1_get_raw(channel);
    }
    adc_reading /= mul_num;
    return esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
}

//DMA模式API待补全
