
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


// 以下为sojson网的 获取合肥近15日天气的API，复制到浏览器地址栏中即可查看（一天内请求过多会被封IP）
// http://t.weather.itboy.net/api/weather/city/101220101
#define WEB_SERVER "t.weather.itboy.net"
#define WEB_PORT "80"
#define WEB_PATH "/api/weather/city/101220101"

static const char *TAG = "example";
// 合成的 HTTP-GET 请求报文，更改了Demo中的Host和User-Agent字段，不要出现WEB_PORT，也不要在User-Agent中出现空格
static const char *REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
    "Host: "WEB_SERVER"\r\n"
    "User-Agent: esp-idf/4.2-esp32\r\n"
    "\r\n";

static void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    //char recv_buf[5120];//char recv_buf[64];
    char *http_rmsg; // http接收的内容
    int max_recv_byte = 1024*8;  // socket单次接收的最大字节数
    http_rmsg = malloc(max_recv_byte); // 申请足够大的内存空间，至少能够存储一次HTTP GET返回的消息

    while(1) {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success:%s", REQUEST);

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        // 在ESP-IDF的Demo基础上，对接收进行了改进升级。能够将以前的socket分段接收合成为一整段，并可统计一帧http报文总长度
        int rnum = 0; // 单次接收报文的总长度
        do {
            r = read(s, http_rmsg+rnum, max_recv_byte-1);
            if (r) // read错误时会返回-1，这种情况不对rnum计数。
                rnum += r;
        } while(r > 0);
        // 输出socket接收的报文
        for(int i = 0; i < rnum; i++) {
            putchar(http_rmsg[i]);
        }
        // 调试输出 r 和 本次接收的报文总长度
        ESP_LOGI(TAG, "r= %d, rnum= %d\n", r, rnum);
        // 从http_rmsg中找到字符 '{'，从此处开始json解析。
        char *json_http = strchr(http_rmsg, '{');
        cjson_parse_sojson_weather(json_http); // cjson解析sojson天气报文
        ESP_LOGW(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size()); // 调试输出空闲内存的空间，及时定位因内存溢出而导致的故障

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
        close(s);
        for(int countdown = 60*60; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(http_get_task, "http_get_task", 4096, NULL, 5, NULL);
}
