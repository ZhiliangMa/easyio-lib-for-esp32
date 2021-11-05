#include "mbedtls_encrypt.h"

static const char *TAG = "mbedtls";

/**
 * @brief  HMAC-SHA1/224/256/384/512 和 HMAC-RIPEMD160/MD2/MD4/MD5 等加密，base64编码。
 * 
 * @param  md_type    加密方式（可选 HMAC-SHA1/224/256/384/512 和 HMAC-RIPEMD160/MD2/MD4/MD5）
 * @param  plain      输入明文
 * @param  key        输入密钥
 * @param  cipher     输出密文，原文
 * @param  cipherHex  输出密文，以十六进制字符串的格式显示【 = cipher->Hex】
 * @param  base64     输出密文，经base64编码后的字符串【 = cipher->base64】
 * 
 * @return
 *     - 0，成功。其他，失败。
 */
int hmac_sha_base64_encrypt(mbedtls_md_type_t md_type, uint8_t *plain, uint8_t *key, uint8_t *cipher, uint8_t *cipherHex, uint8_t *base64)
{
    int ret;
    mbedtls_md_context_t sha_ctx;

    #if tlsEncrypt_Dlog
    ESP_LOGI(TAG, "------------------------------ HMAC 加密、Base64 编码 ---------------\n");
    ESP_LOGI(TAG, "plain: %s", plain);
    ESP_LOGI(TAG, "key: %s", key);
    #endif

    mbedtls_md_init(&sha_ctx);
    ret = mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(md_type), 1);
    if (ret != 0)
    {
        printf("  ! mbedtls_md_setup() returned -0x%04x\n", ret);
        goto exit;
    }
    mbedtls_md_hmac_starts(&sha_ctx, key, strlen((char*)key));
    mbedtls_md_hmac_update(&sha_ctx, plain, strlen((char*)plain));
    mbedtls_md_hmac_finish(&sha_ctx, cipher);
    
    #if tlsEncrypt_Dlog
    // 16进制字符串显示
    switch (md_type) {
        case MBEDTLS_MD_MD2:
            ESP_LOGI(TAG, "HMAC-MD2: ");
            break;
        case MBEDTLS_MD_MD4:
            ESP_LOGI(TAG, "HMAC-MD4: ");
            break;
        case MBEDTLS_MD_MD5:
            ESP_LOGI(TAG, "HMAC-MD5: ");
            break;
        case MBEDTLS_MD_SHA1:
            ESP_LOGI(TAG, "HMAC-SHA1: ");
            break;
        case MBEDTLS_MD_SHA224:
            ESP_LOGI(TAG, "HMAC-SHA224: ");
            break;
        case MBEDTLS_MD_SHA256:
            ESP_LOGI(TAG, "HMAC-SHA256: ");
            break;
        case MBEDTLS_MD_SHA384:
            ESP_LOGI(TAG, "HMAC-SHA384: ");
            break;
        case MBEDTLS_MD_SHA512:
            ESP_LOGI(TAG, "HMAC-SHA512: ");
            break;
        case MBEDTLS_MD_RIPEMD160:
            ESP_LOGI(TAG, "HMAC-RIPEMD160: ");
            break;
        default:
            ESP_LOGI(TAG, "HMAC-None: ");
            break;
    }
    #endif
    
    for (int i = 0; i < strlen((char*)cipher); i++) {
        sprintf((char*)cipherHex+i*2, "%02x", cipher[i]);
    }

    // base64编码
    size_t outlen;
    mbedtls_base64_encode(base64, 29, &outlen, cipher, strlen((char*)cipher));
    #if tlsEncrypt_Dlog
    ESP_LOGI(TAG, "%s\n", cipherHex);
    ESP_LOGI(TAG, "HMAC-HEX: %s\n", cipherHex);
    ESP_LOGI(TAG, "base64: %s", base64);
    #endif
    mbedtls_md_free(&sha_ctx);

exit:
    return ret;
}
