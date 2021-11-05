
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

static const char *TAG = "MQTT";

#define TOPIC_POSTED_PATH  "/topic/post"
#define TOPIC_ISSUED_PATH  "/topic/set"

char clientID[20];
char topic_posted[40] = "/"; // 用于上发设备消息的主题，根据设备MAC自动合成
char topic_issued[40] = "/"; // 用于接收服务器下发消息的主题，根据设备MAC自动合成

// 获取ESP32的MAC，并根据设备MAC自动合成 clientID、topic_posted、topic_issued
void get_device_mac_clientid_topic(void)
{
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac); // 获取ESP32的MAC，6个字节
    //esp_read_mac(mac, ESP_MAC_WIFI_STA); // 同样的获取MAC
    printf("mac: ");
    for (int i=0; i<6; i++)
        printf("%02x ", mac[i]);
    printf("\r\n");
    sprintf(clientID, "ESP32_%.2x%.2x%.2x%.2x", mac[2], mac[3], mac[4], mac[5]); // 使用ESP32自身MAC的后四位，构成client_id
    strcat(topic_posted, clientID);
    strcat(topic_posted, TOPIC_POSTED_PATH); // 使用 "/"+clientID+"/topic/post"，构成用于上发设备消息的主题
    printf("topic_posted: %s\r\n", topic_posted);
    strcat(topic_issued, clientID);
    strcat(topic_issued, TOPIC_ISSUED_PATH); // 使用 "/"+clientID+"/topic/set"，构成用于接收服务器下发消息的主题
    printf("topic_issued: %s\r\n", topic_issued);
}


bool bit_sub_post; // post主题订阅成功标志位。0-未订阅；1-订阅成功。
bool bit_sub_set; // set主题订阅成功标志位。0-未订阅；1-订阅成功。
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id = 0;
    static int post_sub_id = 0; // 订阅post主题的消息ID
    static int set_sub_id = 0;  // 订阅set主题的消息ID
    static char topic_name[50]; // 缓存接收消息的 主题名称，在允许范围内尽量长些
    static char recv_dat[50];   // 缓存接收到的消息，在允许范围内尽量长些
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            // 1.客户端已成功建立与代理的连接。之后可以开始订阅主题
            // 在绝大多数实际应用场景中，一般分别订阅 收/发 主题，以 Qos0 方式订阅
            post_sub_id = esp_mqtt_client_subscribe(client, topic_posted, 0); // 订阅
            ESP_LOGI(TAG, "sent subscribe successful, \"%s\", msg_id=%d", topic_posted, post_sub_id);
            set_sub_id = esp_mqtt_client_subscribe(client, topic_issued, 0);
            ESP_LOGI(TAG, "sent subscribe successful, \"%s\", msg_id=%d", topic_issued, set_sub_id);
            // 订阅后，上发一条测试消息
            msg_id = esp_mqtt_client_publish(client, topic_posted, "Hello!", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
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
            // 解析服务器下发主题 "/topic/set" 的消息，控制led运行状态，并上发回复报文 Done
            if (strstr(topic_name, topic_issued)) {
                if (strstr(recv_dat, "led off")) {
                    vTaskSuspend(led_task_Handler); // 暂停任务，LED停止闪烁
                } else if (strstr(recv_dat, "led on")) {
                    vTaskResume(led_task_Handler); // 继续任务，LED继续闪烁
                }
                msg_id = esp_mqtt_client_publish(client, topic_posted, "Done", 0, 0, 0);
                ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
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
    // 获取ESP32的MAC，并根据设备MAC自动合成 clientID、topic_posted、topic_issued
    get_device_mac_clientid_topic();

    // MQTT客户端配置。如果不额外配置，则默认 port = 1883，keepalive = 120s，开启自动重连，自动重连超时时间为 10s。
    // 不同书写方式的 uri，其默认端口不同，详见：https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/protocols/mqtt.html
    esp_mqtt_client_config_t mqtt_cfg = {
        //.uri = CONFIG_BROKER_URL,             // 使用menuconfig配置项的 MQTT代理URL
        .uri = "mqtt://139.196.254.80",         /*!< Complete MQTT broker URI */
        .client_id = clientID,                  /*!< default client id is ``ESP32_%CHIPID%`` where %CHIPID% are last 3 bytes of MAC address in hex format */
        .username = "easyio",                   /*!< MQTT username */
        .password = "public",                   /*!< MQTT password */
        //.keepalive = 30,                      /*!< mqtt keepalive, default is 120 seconds */
        //.reconnect_timeout_ms = 2000,         /*!< Reconnect to the broker after this value in miliseconds if auto reconnect is not disabled (defaults to 10s) */
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    long i = 0;
    char send_buf[20];
    while(1) {
        // 订阅post成功后，会定时上报计数
        if (bit_sub_post) {
            sprintf(send_buf, "%ld", ++i);
            int msg_id = esp_mqtt_client_publish(client, topic_posted, send_buf, 0, 0, 0);
            ESP_LOGI(TAG, "sent publish, cnt = %ld", i);
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
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
}
