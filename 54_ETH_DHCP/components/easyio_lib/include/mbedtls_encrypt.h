#ifndef __MBEDTLS_ENCRYPTX_H__
#define __MBEDTLS_ENCRYPTX_H__

#include <string.h>
#include "esp_log.h"
#include "mbedtls/pk.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/aes.h"
#include "mbedtls/md5.h"
#include "mbedtls/base64.h"


// tls加密调试信息输出开关，通过串口调试输出 tls加密信息（0关闭，1开启）
#define tlsEncrypt_Dlog    1

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
int hmac_sha_base64_encrypt(mbedtls_md_type_t md_type, uint8_t *plain, uint8_t *key, uint8_t *cipher, uint8_t *cipherHex, uint8_t *base64);

#endif
