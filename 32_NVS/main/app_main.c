
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

#define STORAGE_NAMESPACE    "storage"
#define RESTART_CONTER_KEY   "restart_conter"

/**
 * @brief 从NVS中读取并打印 开机计数值
 */
esp_err_t print_restart_counter(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // 以读写模式，打开命令空间为 storage 的非易失性存储。返回本次操作句柄 my_handle。
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // 读取 开机次数
    int32_t restart_counter = 0; // 如果尚未在NVS中设置，则该值将默认为0
    err = nvs_get_i32(my_handle, RESTART_CONTER_KEY, &restart_counter);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    printf("Restart counter = %d\n", restart_counter);

    // 关闭NVS
    nvs_close(my_handle);
    return ESP_OK;
}

/**
 * @brief 在NVS中保存MCU的开机次数（每次开机后调用，来记录开机次数）
 */
esp_err_t save_restart_counter(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // 以读写模式，打开命令空间为 storage 的非易失性存储。返回本次操作句柄 my_handle。
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // 读取 开机次数
    int32_t restart_counter = 0; // 如果尚未在NVS中设置，则该值将默认为0
    err = nvs_get_i32(my_handle, RESTART_CONTER_KEY, &restart_counter);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    // ++后重新写入
    restart_counter++;
    err = nvs_set_i32(my_handle, RESTART_CONTER_KEY, restart_counter);
    if (err != ESP_OK) return err;

    // 关闭NVS前，必须调用nvs_commit（）以确保将更改写入闪存
    err = nvs_commit(my_handle);
    if (err != ESP_OK) return err;

    // 关闭NVS
    nvs_close(my_handle);
    return ESP_OK;
}

// nvs_storage_task 任务。初始化 NVS，来记录总开机上电次数。
void nvs_storage_task(void* arg)
{
    // 初始化默认的NVS分区。
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS分区已被截断，需要删除
        // 重试 nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    // 从NVS中读取并打印 开机计数值
    err = print_restart_counter();
    if (err != ESP_OK) printf("Error (%s) reading data from NVS!\n", esp_err_to_name(err));

    // 在NVS中保存MCU的开机次数（每次开机后调用，来记录开机次数）
    err = save_restart_counter();
    if (err != ESP_OK) printf("Error (%s) saving restart counter to NVS!\n", esp_err_to_name(err));

    while(1) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    // 创建 nvs_storage_task 任务。
    xTaskCreate(nvs_storage_task, "nvs_storage_task", 2048, NULL, 3, NULL);
}
