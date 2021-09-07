# 11_ADC

## 例程简介

介绍ESP32的ADC1的使用。（由于ADC2不能与WIFI共用，所以尽量优先使用ADC1，且ADC2的读取方式与ADC1不同，也就没有在esayIO中提供ADC2的初始化和读取函数。况且ADC1已经满足绝大多数场景使用。）

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **adc1_scan_task** ，扫描ADC1的`ADC_CHANNEL_3`通道：电路为光敏电阻。并控制led任务运行状态

使用 `adc_sampling.c.h` 驱动模块，来对ESP32的 `ADC1` 进行配置。

鉴于ESP32的ADC校准补偿比较麻烦，且注意事项过多，不建议了解硬件细节，直接使用easyIO封装好的函数去调用。


## 运行现象

* 串口打印输出 `光敏电阻信号 - ADC_CHANNEL_3` 的电压值。

* 用手遮挡光敏电阻，观察到ADC1通道x输出的值变小，LED继续闪烁。处于明亮环境，LED暂停闪烁。


## 学习内容

1. ADC1的通道、衰减、读取函数调用。

2. ESP32的ADC硬件没有模拟地，故噪声较大，且在量程的两头为非线性。如何在这种情况下提高ADC的实用性？：使用多重采样去减低噪声、尽量去测量读数在量程中间范围的模拟量

3. easyIO最多支持8路ADC的输入，且不支持任意GPIO映射，引脚映射表在`adc_sampling.c.h`文件中。


## 关键函数

```c
// ADC1及输入通道初始化（在特定衰减下表征ADC的特性，并生成ADC电压曲线）
void adc1_init_with_calibrate(adc_atten_t atten, int ch_num, ...);

// 获取ADC1通道x转换后的原始值
int adc1_get_raw(adc1_channel_t channel);

// 获取ADC1通道x经校准补偿后的转换电压，单位mV
uint32_t adc1_cal_get_voltage(adc_channel_t channel);

// 获取ADC1通道x，经多重采样平均后，并校准补偿后的转换电压，单位mV
uint32_t adc1_cal_get_voltage_mul(adc_channel_t channel, uint32_t mul_num);
```


## 注意事项

- 由于`ADC2`不能与`WIFI`共用，所以尽量优先使用ADC1，且ADC2的读取方式与ADC1不同，也就没有在esayIO中提供ADC2的初始化和读取函数。况且ADC1已经满足绝大多数场景使用，没必要因为ADC2给驱动库编写带来麻烦。

- TP两点校准值是用户自己测量，并刻录到`eFuse`中。而`eFuse Vref`由工厂生产时刻录。（就是说芯片刚出厂只会有Vref）

- 不同的衰减系数，会影响输入电压量程，但会影响经校准补偿后的值。且衰减越大，对读数准确性的影响也越大。尽量使用 `较低的衰减系数`，以获得更高的测量精度。

- 使用 adc1_init_with_calibrate 去配置ADC，并不会对ADC转换后的值产生影响。

- `adc1_get_raw` 用来读取`原始转换值`，`adc1_cal_get_voltage` 用来读取`经校准补偿后的电压值`，`adc1_cal_get_voltage_mul` 用来读取`多重采样并校准补偿后的电压值`。

- ESP32的ADC的每个通道的衰减`好像`都可以单独配置，但不同的衰减需要匹配的不同的校准补偿结构体 `esp_adc_cal_characteristics_t`，会给驱动编写带来麻烦，故easyIO留出的API中，将所有通道的衰减都设为了相同值。

- 在初始化SARADC1 或SARADC2 或霍尔传感器的时候，会在SENSOR_VP/SENSOR_VN PAD 的内部产生一个持续约80 ns 的输入毛刺。如果设计中使用了SENSOR_VP/SENSOR_VN 作为GPIO，且同时使用了其他ADC，则需要额外的软件处理，避开该毛刺。

- 优先推荐使用SENSOR_VP 及SENSOR_VN 作为ADC 使用。

- 目前不支持高精度ADC。SENSOR_VP 和SENSOR_CAPP 及SENSOR_VN 和SENSOR_CAPN 之间的两个采样电容270 pF 可删除。之后这4 个管脚可当做普通的ADC 或是GPIO 使用。

- ADC 的输入电压建议不超过2450 mV，推荐在100 ~ 950 mV 间，以获得更高的校准精度。

- ESP32硬件没有AGND模拟地。

- IDF V4.2和4.3不一样。
- 非线性，要到0.1xV，才会有读数。对精准度有严格要求的建议用外接的独立ADC。

- ESP-IDF V4.2与V4.3在 ADC上对源文件的引用有区别。V4.3的`CMakeLists.txt` 需要增加 `"${IDF_PATH}/components/esp_adc_cal/esp_adc_cal_esp32.c"`。

- ADC的DMA模式的API待补全。
