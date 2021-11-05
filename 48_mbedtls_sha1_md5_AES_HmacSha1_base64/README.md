# 48_mbedtls_sha1_md5_AES_HmacSha1_base64

## 例程简介

本节Demo主要为后续`MQTT连接阿里云物联网平台`做准备，连接大厂物联网平台，clientid、username、passwd 都需要加密算法加密，以保证安全性。

[阿里云物联网平台-接入文档](https://help.aliyun.com/document_detail/140507.html)

ESP-IDF 内置了 `mbedtls` 开源加密算法库，可以直接输入明文，由ESP32自身计算密文。且因为 `mbedtls` 为开源库，使用方法与其他硬件平台都是一样的。

ARM MBED 在线API文档：[mbedtls-API文档-base64](https://os.mbed.com/teams/sandbox/code/mbedtls/docs/tip/base64_8h.html)

mbed TLS（以前称为PolarSSL）是TLS和SSL协议的实现，mbedtls主要包括密码学算法、X.509证书、TLS/DTLS协议3个组成部分。mbedtls在嵌入式系统中可作为OpenSSL的替代者。

ARM `mbedtls`使开发人员可以非常轻松地在`嵌入式`产品中加入加密和 SSL/TLS 功能。

使用 `mbedtls` 常用的 sha、HmacSha、MD5、AES 加密算法库，仅需包含头文件
```c
#include "mbedtls/pk.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha1.h"
#include "mbedtls/aes.h"
#include "mbedtls/md5.h"
#include "mbedtls/base64.h"
```

`easyio`其中将比较常用 HMAC-SHA1/224/256/384/512 和 HMAC-RIPEMD160/MD2/MD4/MD5 等加密，base64编码。进行了封装，可看下一Demo。`MQTT连接阿里云物联网平台，上报温湿度、光照度、电池电压，下发命令控制LED运行状态`

推荐一在线 计算sha、HmacSha、MD5、AES、DES 的网页，用于比对：[在线加密工具](https://www.mklab.cn/utils/sha)
【上面工具的AES计算不对，可见：[AES在线加密解密](http://tool.chacuo.net/cryptaes)】

推荐一类似的在线计算`HMAC`工具，且有 base64编码：[1024程序员开发工具箱-在线加密工具](https://1024tools.com/hmac) 【注意，这款的`sha`实际是`HmacSha`，切记不要弄错。base64编码在接入阿里云MQTT消息队列时要用，物联网平台不用】

此Demo代码参照，特此感谢：[徐宏-乐鑫Esp32学习之旅](https://blog.csdn.net/xh870189248/article/details/103539421?utm_medium=distribute.pc_aggpage_search_result.none-task-blog-2~aggregatepage~first_rank_ecpm_v1~rank_aggregation-1-103539421.pc_agg_rank_aggregation&utm_term=%E4%B9%90%E9%91%ABESP32%E7%9A%84md5%E5%8A%A0%E5%AF%86&spm=1000.2123.3001.4430)

***

有兴趣的话，可以提前了解下：[阿里云物联网平台-登录三元组的合成方式、网络调试助手 逐字节讲解MQTT接入](https://blog.csdn.net/Mark_md/article/details/108314817?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163523050816780366596744%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163523050816780366596744&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29_name-1-108314817.pc_v2_rank_blog_default&utm_term=%E9%98%BF%E9%87%8C%E4%BA%91&spm=1018.2226.3001.4450)

使用MQTT.fx模拟接入更为方便：[MQTT.fx客户端接入阿里云物联网平台](https://blog.csdn.net/Mark_md/article/details/108316694?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522163523050816780366596744%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=163523050816780366596744&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29_name-4-108316694.pc_v2_rank_blog_default&utm_term=%E9%98%BF%E9%87%8C%E4%BA%91&spm=1018.2226.3001.4450)

***

## 运行现象

Demo测试了 `mbedtls` 加密算法库的 `SHA-1加密`、`SHA-256加密`、`MD5加密`、`AES-ECB 加解密`、`HMAC加密`、`Base64编码`。

1. 编译、下载。

2. 根据在线计算工具，对比ESP32设备输出的密文是否一致。

```
I (0) cpu_start: App cpu up.
I (241) heap_init: Initializing. RAM available for dynamic allocation:
I (248) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (254) heap_init: At 3FFB28E8 len 0002D718 (181 KiB): DRAM
I (260) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (267) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (273) heap_init: At 40089FEC len 00016014 (88 KiB): IRAM
I (279) cpu_start: Pro cpu start user code
I (298) spi_flash: detected chip: generic
I (298) spi_flash: flash io: dio
I (298) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (0) gpio: GPIO[33]| InputEn: 1| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (701) mbedtls: ------------------------------ SHA-1 加密测试 ------------------------------

I (701) mbedtls: Sha1 要加密数据: https://github.com/ZhiliangMa/easyio-lib-for-esp32
I (701) mbedtls: Sha1 加密后数据:
574c30f1f69524090dde6691e3733b7765b74049

I (711) mbedtls: ------------------------------ SHA-256 加密测试 ----------------------------

I (721) mbedtls: Sha256 要加密数据: https://github.com/ZhiliangMa/easyio-lib-for-esp32
I (731) mbedtls: Sha256 加密后数据: 
50176fe6af5c2866b0e71f316401002e30ac45cd4f6072dd28049c253cde3436

I (741) mbedtls: ------------------------------ MD5 加密测试 --------------------------------

I (751) mbedtls: Md5加密前: https://github.com/ZhiliangMa/easyio-lib-for-esp32
 md5加密后(32位):
2378de3d8193dbbe6508fccf38fb3a80

I (761) mbedtls: ------------------------------ AES-ECB 加解密测试 --------------------------

I (771) mbedtls: AES-ECB 加密-数据块(128位)，偏移量为0
I (781) mbedtls: 要加密的数据: easyio-iot.com
I (791) mbedtls: 加密的密码: passwd
I (791) mbedtls: 加密结果，十六进制表示: 
2e0c36b7b3d7070e7789ada4cee00b3c
I (801) mbedtls: 解密后的数据: easyio-iot.com


I (801) mbedtls: ------------------------------ HMAC 加密测试、Base64 编码测试 ---------------

I (811) mbedtls: 要加密的数据: easyio-iot.com
I (821) mbedtls: 加密的密码: passwd
HMAC: b6afb974e9b8c21b710dbd768237ae069ae456ff
base64: tq+5dOm4whtxDb12gjeuBprkVv8=

```


## 注意事项

* mbedtls 的 AES-ECB模式，只能实现最大16字节的明文加解密。

* X.509证书、https的例程存在于`ESP-IDF`的Demo列表内。本例程仅对 `mbedtls` 常用的 sha、HmacSha、MD5、AES 加密算法 和 Base64编码进行测试。
