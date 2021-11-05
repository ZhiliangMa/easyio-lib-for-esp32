
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

static const char *TAG = "mbedtls";

// sha1加密测试
void sha1_test(void)
{
    unsigned char decrypt[32];
    const unsigned char encrypt[] = "https://github.com/ZhiliangMa/easyio-lib-for-esp32";

    ESP_LOGI(TAG, "------------------------------ SHA-1 加密测试 ------------------------------\n");
    ESP_LOGI(TAG, "Sha1 要加密数据: %s", encrypt);

    mbedtls_sha1_context sha1_ctx;
    mbedtls_sha1_init(&sha1_ctx);
    mbedtls_sha1_starts(&sha1_ctx);
    mbedtls_sha1_update(&sha1_ctx, encrypt, strlen((char *)encrypt));
    mbedtls_sha1_finish(&sha1_ctx, decrypt);
    mbedtls_sha1_free(&sha1_ctx);

    ESP_LOGI(TAG, "Sha1 加密后数据:");
    for (int i = 0; i < 20; i++)
    {
        printf("%02x", decrypt[i]);
    }

    printf("\n\n");
}

// sha256/224加密测试
void sha256_test(void)
{
    unsigned char decrypt[32];
    const unsigned char encrypt[] = "https://github.com/ZhiliangMa/easyio-lib-for-esp32";

    ESP_LOGI(TAG, "------------------------------ SHA-256 加密测试 ----------------------------\n");
    // sha256/224
    ESP_LOGI(TAG, "Sha256 要加密数据: %s", encrypt);

    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0); // 0表示传sha256 ， 1 表示传SHA-224
    mbedtls_sha256_update(&sha256_ctx, encrypt, strlen((char *)encrypt));
    mbedtls_sha256_finish(&sha256_ctx, decrypt);
    ESP_LOGI(TAG, "Sha256 加密后数据: ");
    for (int i = 0; i < 32; i++)
    {
        printf("%02x", decrypt[i]);
    }
    mbedtls_sha256_free(&sha256_ctx);
    printf("\n\n");
}

// MD5加密测试
void md5_test(void)
{
    unsigned char encrypt[] = "https://github.com/ZhiliangMa/easyio-lib-for-esp32";
    unsigned char decrypt[16];

    ESP_LOGI(TAG, "------------------------------ MD5 加密测试 --------------------------------\n");

    mbedtls_md5_context md5_ctx;
    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);
    mbedtls_md5_update(&md5_ctx, encrypt, strlen((char *)encrypt));
    mbedtls_md5_finish(&md5_ctx, decrypt);
    ESP_LOGI(TAG, "Md5加密前: %s\n md5加密后(32位):", encrypt);

    for (int i = 0; i < 16; i++)
    {
        printf("%02x", decrypt[i]);
    }
    mbedtls_md5_free(&md5_ctx);

    printf("\n\n");
}

// AES-ECB加解密测试
// ECB加密解密：数据块128位 偏移量为0，无填充
// 【注意】mbedtls 的 AES-ECB模式，只能实现最大16字节的明文加解密。 !!!
void aes_ecb_test(void)
{
    //明文空间
    unsigned char plain[16] = "easyio-iot.com";
    //密钥
    unsigned char key[16] = "passwd";
    //密文空间
    unsigned char cipher[16] = {0};
    //解密后明文的空间
    unsigned char dec_plain[16] = {0};

    ESP_LOGI(TAG, "------------------------------ AES-ECB 加解密测试 --------------------------\n");
    ESP_LOGI(TAG, "AES-ECB 加密-数据块(128位)，偏移量为0");

    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);
    //设置加密密钥
    mbedtls_aes_setkey_enc(&aes_ctx, key, 128);
    ESP_LOGI(TAG, "要加密的数据: %s", plain);
    ESP_LOGI(TAG, "加密的密码: %s", key);
    mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, plain, cipher);
    ESP_LOGI(TAG, "加密结果，十六进制表示: ");
    for (int i = 0; i < 16; i++)
        printf("%02x", cipher[i]);
    printf("\r\n");

    //设置解密密钥
    mbedtls_aes_setkey_dec(&aes_ctx, key, 128);
    mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, cipher, dec_plain);
    ESP_LOGI(TAG, "解密后的数据: %s", dec_plain);
    mbedtls_aes_free(&aes_ctx);

    printf("\n\n");
}

// AES-CBC没验证对，不做列举

// HMAC加密测试、Base64编码测试
// 可选 HMAC-SHA1、HMAC-SHA256、HMAC-SHA512、HMAC-MD5 等，并附带 base64 编码输出
int hmac_sha_base64_test(mbedtls_md_type_t md_type)
{
    int ret;
    //unsigned char plain[] = "easyio-iot.com"; // 明文消息
    //unsigned char key[] = "passwd"; // 密钥
    unsigned char plain[] = "clientIdco_0001deviceNameco_0001productKeya1wFylTxYeD"; // 明文消息
    unsigned char key[] = "7ab0c4b3532b5783df5fdc58a2895d7a"; // 密钥
    unsigned char cipher[32] = {0}; // 密文
    mbedtls_md_context_t sha_ctx;

    ESP_LOGI(TAG, "------------------------------ HMAC 加密测试、Base64 编码测试 ---------------\n");
    ESP_LOGI(TAG, "要加密的数据: %s", plain);
    ESP_LOGI(TAG, "加密的密码: %s", key);

    mbedtls_md_init(&sha_ctx);
    //memset(cipher, 0x00, sizeof(cipher));
    ret = mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(md_type), 1);
    if (ret != 0)
    {
        printf("  ! mbedtls_md_setup() returned -0x%04x\n", ret);
        goto exit;
    }
    mbedtls_md_hmac_starts(&sha_ctx, key, sizeof(key) - 1); // 如果改为strlen，就不用-1
    mbedtls_md_hmac_update(&sha_ctx, plain, sizeof(plain) - 1);
    mbedtls_md_hmac_finish(&sha_ctx, cipher );

    printf("HMAC: ");
    for (int i = 0; i < strlen((char*)cipher); i++)
        printf("%02x", cipher[i]);
    printf("\n");
    // base64编码
    unsigned char cipher_base64[64] = {0}; // 密文
    size_t outlen;
    mbedtls_base64_encode(cipher_base64, sizeof(cipher_base64), &outlen, cipher, strlen((char*)cipher));
    printf("base64: %s", cipher_base64);
    printf("\n\n");
    mbedtls_md_free(&sha_ctx);

exit:
    return ret;
}

// mbedtls_test 任务。测试 mbedtls 开源加密算法库，计算 sha、HmacSha、MD5 的结果
void mbedtls_test(void* arg)
{
    vTaskDelay(200 / portTICK_PERIOD_MS);

    sha1_test(); // sha1加密测试
    sha256_test(); // sha256/224加密测试
    md5_test(); // MD5加密测试
    aes_ecb_test(); // AES-ECB加解密测试
    hmac_sha_base64_test(MBEDTLS_MD_SHA1); // HMAC加密测试、Base64编码测试

    vTaskDelete(NULL);
}

void app_main(void)
{
    // 创建 led_task 任务，任务栈空间大小为 2048，任务优先级为3（configMINIMAL_STACK_SIZE 不能用 printf，会导致ESP32反复重启）
    xTaskCreate(led_task, "led_task", 2048, NULL, 3, &led_task_Handler);

    xTaskCreate(mbedtls_test, "mbedtls_test", 4096, NULL, 5, NULL);
}
