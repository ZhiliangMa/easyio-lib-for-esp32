
#include "easyio.h"

#define LED 33
#define KEY 0
#define KEY_USR 27

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

static const char *TAG = "MQTT";

/*
{
  "ProductKey": "a1WFMPFCsrp",
  "DeviceName": "dev_0001",
  "DeviceSecret": "2e44554a1e557488ea2b772bf1d79c87"
}
*/
// 设备证书（ProductKey、DeviceName和DeviceSecret），三元组
/*char productKey[20] = "a1WFMPFCsrp";
char deviceName[20] = "dev_0001";
char deviceSecret[40] = "2e44554a1e557488ea2b772bf1d79c87"; // 32字节长度
char regionId[20] = "cn-shanghai"; // 接入的服务器地址*/

/*
{
  "ProductKey": "a1WFMPFCsrp",
  "DeviceName": "dev_0002",
  "DeviceSecret": "605ed1663e9d945e014d462e8f201e98"
}
*/
// 设备证书（ProductKey、DeviceName和DeviceSecret），三元组
char productKey[20] = "a1WFMPFCsrp";
char deviceName[20] = "dev_0002";
char deviceSecret[40] = "605ed1663e9d945e014d462e8f201e98"; // 32字节长度
char regionId[20] = "cn-shanghai"; // 接入的服务器地址


char mqttClientId[60] = {0};
char mqttUsername[60] = {0};
char mqttPassword[50] = {0};
char brokerUrl[60] = {0};  // MQTT 接入点域名
char topic_post[60] = {0}; // Topic，post，客户端向服务器上报消息的主题
char topic_set[60] = {0};  // Topic，set，服务器向客户端下发消息的主题

// SHT30温湿度传感器的数据
uint8_t recv_dat[6] = {0};
float temperature = 0.0;
float humidity = 0.0;

// ADC采集板载光照强度、电池电压
int ligehLux = 0;
float v_bat = 0.0;

// led状态灯运行状态
int statusLight = 1; // 0-停止闪烁；1-闪烁。

bool bit_sub_post; // post主题订阅成功标志位。0-未订阅；1-订阅成功。
bool bit_sub_set; // set主题订阅成功标志位。0-未订阅；1-订阅成功。
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id = 0;
    static int post_sub_id = 0; // 订阅post主题的消息ID
    static int set_sub_id = 0;  // 订阅set主题的消息ID
    static char topic_name[60]; // 缓存接收消息的 主题名称，在允许范围内尽量长些
    static char recv_dat[1000];   // 缓存接收到的消息，在允许范围内尽量长些
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            // 1.客户端已成功建立与代理的连接。之后可以开始订阅主题
            // 在绝大多数实际应用场景中，一般分别订阅 收/发 主题，以 Qos0 方式订阅
            post_sub_id = esp_mqtt_client_subscribe(client, topic_post, 0); // 订阅
            ESP_LOGI(TAG, "sent subscribe successful, \"%s\", msg_id=%d", topic_post, post_sub_id);
            set_sub_id = esp_mqtt_client_subscribe(client, topic_set, 0);
            ESP_LOGI(TAG, "sent subscribe successful, \"%s\", msg_id=%d", topic_set, set_sub_id);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            // 2.代理服务器已确认客户端的订阅请求。
            // 这里不能由 event 直接获得主题名称，需要由 msg_id 来判断先前的哪条主题已被订阅
            // 置位订阅成功标志位。主任务检测到订阅成功后，会定时上发消息
            if (event->msg_id == post_sub_id) {
                bit_sub_post = 1;
            } else if (event->msg_id == set_sub_id) {
                bit_sub_set = 1;
            }
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            // 3.客户端已收到发布的消息（客户端自己发布的消息，也会因为主题的广播机制而再接收到一次）。调试输出 接收到消息的主题、数据、数据长度
            ESP_LOGI(TAG, "TOPIC = \"%.*s\"\r\n", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA = \"%.*s\", num = %d\r\n", event->data_len, event->data, event->data_len);

            // 转移主题名称和数据，用于后续解析
            memcpy(topic_name, event->topic, event->topic_len);
            memcpy(recv_dat, event->data, event->data_len);
            int ret = cjson_parse_aliyun_iot_StatusLightSwitch(recv_dat); // cjson解析阿里云物联网平台的 `状态灯开关` 控制消息
            if (ret != -1) { // 判断解析是否有效
                if (ret) {
                    vTaskResume(led_task_Handler); // 继续任务，LED继续闪烁
                    statusLight = 1;
                } else {
                    vTaskSuspend(led_task_Handler); // 暂停任务，LED停止闪烁
                    statusLight = 0;
                    led_set(LED, 0);
                }
            }
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

// MQTT事件处理回调
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start_task(void *arg)
{
    // 输入设备证书（三元组）和接入的服务器地址，函数会自动计算并输出 MQTT认证所需的 mqttClientId、mqttUsername、mqttPassword、brokerUrl，以及设备属性/设置的主题
    aliyun_iot_device_clientKey_generate(productKey, deviceName, deviceSecret, regionId,
                                         mqttClientId, mqttUsername, mqttPassword, brokerUrl, topic_post, topic_set);

    // MQTT客户端配置。如果不额外配置，则默认 port = 1883，keepalive = 120s，开启自动重连，自动重连超时时间为 10s。
    // 不同书写方式的 uri，其默认端口不同，详见：https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/protocols/mqtt.html
    esp_mqtt_client_config_t mqtt_cfg = {
        //.uri = CONFIG_BROKER_URL,             // 使用menuconfig配置项的 MQTT代理URL
        .host = brokerUrl,                      /*!< Complete MQTT broker URI */
        .port = 1883,                           /*!< MQTT server port */
        .client_id = mqttClientId,              /*!< default client id is ``ESP32_%CHIPID%`` where %CHIPID% are last 3 bytes of MAC address in hex format */
        .username = mqttUsername,               /*!< MQTT username */
        .password = mqttPassword,               /*!< MQTT password */
        //.keepalive = 30,                      /*!< mqtt keepalive, default is 120 seconds */
        //.reconnect_timeout_ms = 2000,         /*!< Reconnect to the broker after this value in miliseconds if auto reconnect is not disabled (defaults to 10s) */
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    while(1) {
        // 订阅post成功后，会定时上报SHT30传感器的温湿度数据
        if (bit_sub_post) {
            char *payload;
            //payload = aliyun_iot_Temperature_Humidity_json_splice(temperature, (int)humidity); // 合成上报温湿度传感器信息的json字段
            //payload = aliyun_iot_Temperature_Humidity_mlux_Voltage_json_splice(temperature, (int)humidity, ligehLux, v_bat); // 合成上报温湿度传感器信息的json字段
            payload = aliyun_iot_Temperature_Humidity_mlux_Voltage_statusLight_json_splice(temperature, (int)humidity, ligehLux, v_bat, statusLight); // 合成上报温湿度传感器信息的json字段
            int msg_id = esp_mqtt_client_publish(client, topic_post, payload, 0, 0, 0); // 发布消息
            cJSON_free((void *) payload); // 释放cJSON_Print ()分配出来的内存空间
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        ESP_LOGW(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size()); // 调试输出空闲内存的空间，及时定位因内存溢出而导致的故障
    }
}

// i2c_sht30_task 任务。初始化 SHT30工作于周期测量模式，获取环境温湿度数据
void i2c_sht30_task(void* arg)
{
    // 配置I2C0-主机模式，400K，指定 SCL-14，SDA-4
    i2c_master_init(I2C_NUM_0, 400000, GPIO_NUM_14, GPIO_NUM_4);

    SHT30_reset(); // 复位SHT30
    if(SHT30_Init() == ESP_OK) // 初始化SHT30(周期测量模式)
        ESP_LOGI(TAG, "sht30 init ok.\n");
    else
        ESP_LOGE(TAG, "sht30 init fail.\n");

    while(1) {
        if(SHT30_Read_Dat(recv_dat) == ESP_OK) { // 从SHT30读取一次数据（周期测量模式下）
            // 将SHT30接收的6个字节数据进行CRC校验，并转换为温度值和湿度值
            if(SHT30_Dat_To_Float(recv_dat, &temperature, &humidity)==0) {
                ESP_LOGI(TAG, "temperature = %.2f ℃, humidity = %.2f %%\n", temperature, humidity);
            } else {
                ESP_LOGE(TAG, "crc check fail.\n");
            }
        } else {
            ESP_LOGE(TAG, "read data from sht30 fail.\n");
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// adc1_scan_task 任务，扫描ADC1的两个2个通道：电位器、光敏电阻。并控制led任务运行状态
void adc1_scan_task(void* arg)
{
    //ADC1及输入通道初始化
    //不同的衰减系数，会影响经校准补偿后的值。且衰减越大，对读数准确性的影响也越大。
    //不同的衰减系数，对应的ESP32引脚输入范围为：0：75~1008mV。2.5：78~1317mV。6：107~1821mV。11：142~3108mV。
    //超过衰减系数的电压输入范围，可能导致ADC损毁。可以通过设置更高的衰减来扩展输入范围，但推荐使用更低的衰减以提高校准后值的准确性。
    //adc1_init_with_calibrate(ADC_ATTEN_DB_2_5, 1, ADC_CHANNEL_6);
    //adc1_init_with_calibrate(ADC_ATTEN_DB_11, 1, ADC_CHANNEL_3);
    adc1_init_with_calibrate(ADC_ATTEN_DB_11, 2, ADC_CHANNEL_0, ADC_CHANNEL_3);

    // 配置ADC_CON为推挽输出，设置初始电平为1，开通光敏电路和电池分压采样电路的供电（ADC闲时，ADC_CON可设置输出为0，关断电压输出，以降低分压电阻上功耗）
    gpiox_set_ppOutput(12, 1);

    while (1) {
        gpio_set_level(12, 1); // 开通 光照传感器、电池电压测量电路的电源
        vTaskDelay(20 / portTICK_PERIOD_MS); // 延时20ms，待电压稳定
        //ADC读取光敏传感器的电压，经64次多重采样并校准补偿后的电压值
        ligehLux = adc1_cal_get_voltage_mul(ADC_CHANNEL_3, 64);
        //ADC读取电池电压，获取到读数后乘分压比例系数，得到实际电压。
        v_bat = adc1_cal_get_voltage_mul(ADC_CHANNEL_0, 64) * 5.7;
        v_bat /= 1000;
        ESP_LOGI(TAG, "ligehLux: %dmV\tv_bat: %fV\n", ligehLux, v_bat);

        gpio_set_level(12, 0); // 关闭 光照传感器、电池电压测量电路的电源，降低闲时功耗
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// key_scan_task 任务，单独运行一个任务，运行按键检测（滤除按键抖动、短按、长按）
void key_scan_task(void* arg)
{
    //按键检测任务
    //key_scan(1, KEY); // 1个按键
    key_scan(2, KEY, KEY_USR); // 2个按键
}

// key_catch_task 任务。去捕获按键事件，并控制LED任务状态。
void key_catch_task(void* arg)
{
    uint32_t key_event = 0;
    uint32_t key_gpio = 0; // 触发按键事件的按键gpio序号
    uint32_t key_type = 0; // 触发按键事件的类型

    while(1) {
        // 以阻塞方式，不断读取 key_evt_queue 队列信息，监控按键事件
        if (xQueueReceive(key_evt_queue, &key_event, portMAX_DELAY)) {
            // 拆分按键事件，得到按键值和按键类型
            key_gpio = key_event & 0x0000FFFF; // 按键值
            key_type = key_event >> 16; // 按键类型（1为短按，2为长按）

            ESP_LOGW(TAG, "key:%d, type:%d\n", key_gpio, key_type); // 输出按键事件

            if(key_gpio == KEY) { // 使用 KEY 控制LED运行状态
                if(key_type == 1) { // 1，短按，暂停LED任务
                    vTaskSuspend(led_task_Handler); // 暂停任务，LED停止闪烁
                    statusLight = 0;
                    led_set(LED, 0);
                }else if(key_type == 2) { // 2，长按，继续LED任务
                    vTaskResume(led_task_Handler); // 继续任务，LED继续闪烁
                    statusLight = 1;
                }
            }
        }
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(mqtt_app_start_task, "mqtt_app_start_task", 4096, NULL, 5, NULL);

    // 创建 i2c_sht30_task 任务。初始化 SHT30工作于周期测量模式，获取环境温湿度数据
    xTaskCreate(i2c_sht30_task, "i2c_sht30_task", 2048, NULL, 3, NULL);
    // 创建 adc1_scan_task 任务，运行任务栈空间大小为 2048，任务优先级为3。
    xTaskCreate(adc1_scan_task, "adc1_scan_task", 2048, NULL, 3, NULL);
    // 创建 key_scan_task 任务，运行任务栈空间大小为 4096，任务优先级为3。实测分配小于2048会导致开机反复重启
    xTaskCreate(key_scan_task, "key_scan_task", 4096, NULL, 3, NULL);
    // 创建 key_catch_task 任务，任务栈空间大小为 4096，任务优先级为3。去捕获按键事件，并控制LED任务状态。
    xTaskCreate(key_catch_task, "key_catch_task", 4096, NULL, 2, NULL);
}
