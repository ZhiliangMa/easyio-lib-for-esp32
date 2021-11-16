# 50_SNTP_DeepSleep

## 一、例程简介

此例程摘自IDF的`SCAN`，用来扫描可用的 AP 集（WIFI热点）。

`CONFIG_EXAMPLE_SCAN_LIST_SIZE` 参数需要从示例配置菜单中设置。 它表示驱动程序可以填充的最大扫描列表条目数。


## 二、运行现象

```
I (0) cpu_start: App cpu up.
I (422) heap_init: Initializing. RAM available for dynamic allocation:
I (429) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (435) heap_init: At 3FFB77E8 len 00028818 (162 KiB): DRAM
I (441) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (448) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (454) heap_init: At 400955E0 len 0000AA20 (42 KiB): IRAM
I (460) cpu_start: Pro cpu start user code
I (479) spi_flash: detected chip: generic
I (479) spi_flash: flash io: dio
I (480) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (538) wifi:wifi driver task: 3ffbfea4, prio:23, stack:6656, core=0
I (538) system_api: Base MAC address is not set
I (538) system_api: read default base MAC address from EFUSE
I (548) wifi:wifi firmware version: bb6888c
I (548) wifi:wifi certification version: v7.0
I (548) wifi:config NVS flash: enabled
I (548) wifi:config nano formating: disabled
I (558) wifi:Init data frame dynamic rx buffer num: 32
I (558) wifi:Init management frame dynamic rx buffer num: 32
I (568) wifi:Init management short buffer num: 32
I (568) wifi:Init dynamic tx buffer num: 32
I (578) wifi:Init static rx buffer size: 1600
I (578) wifi:Init static rx buffer num: 10
I (578) wifi:Init dynamic rx buffer num: 32
I (588) wifi_init: rx ba win: 6
I (588) wifi_init: tcpip mbox: 32
I (598) wifi_init: udp mbox: 6
I (598) wifi_init: tcp mbox: 6
I (598) wifi_init: tcp tx win: 5744
I (608) wifi_init: tcp rx win: 5744
I (608) wifi_init: tcp mss: 1440
I (618) wifi_init: WiFi IRAM OP enabled
I (618) wifi_init: WiFi RX IRAM OP enabled
I (628) phy_init: phy_version 4660,0162888,Dec 23 2020
I (728) wifi:mode : sta (08:3a:f2:4a:3f:88)
I (2838) scan: Total APs scanned = 9
I (2838) scan: SSID             CMCC-vVSQ
I (2838) scan: RSSI             -66
I (2838) scan: Authmode         WIFI_AUTH_WPA_WPA2_PSK
I (2838) scan: Pairwise Cipher  WIFI_CIPHER_TYPE_TKIP_CCMP
I (2848) scan: Group Cipher     WIFI_CIPHER_TYPE_TKIP
I (2848) scan: Channel          1

I (2858) scan: SSID             CMCC-eear
I (2858) scan: RSSI             -69
I (2858) scan: Authmode         WIFI_AUTH_WPA_WPA2_PSK
I (2868) scan: Pairwise Cipher  WIFI_CIPHER_TYPE_TKIP_CCMP
I (2878) scan: Group Cipher     WIFI_CIPHER_TYPE_TKIP
I (2878) scan: Channel          4

I (2878) scan: SSID             cc
I (2888) scan: RSSI             -71
I (2888) scan: Authmode         WIFI_AUTH_WPA_WPA2_PSK
I (2898) scan: Pairwise Cipher  WIFI_CIPHER_TYPE_CCMP
I (2898) scan: Group Cipher     WIFI_CIPHER_TYPE_CCMP
I (2908) scan: Channel          2

I (2908) scan: SSID             ChinaNet-JeNR
I (2908) scan: RSSI             -71
I (2918) scan: Authmode         WIFI_AUTH_WPA_WPA2_PSK
I (2918) scan: Pairwise Cipher  WIFI_CIPHER_TYPE_TKIP_CCMP
I (2928) scan: Group Cipher     WIFI_CIPHER_TYPE_TKIP
I (2928) scan: Channel          4

I (2938) scan: SSID             勇敢牛牛，不怕猪猪
I (2938) scan: RSSI             -73
I (2948) scan: Authmode         WIFI_AUTH_WPA_WPA2_PSK
I (2948) scan: Pairwise Cipher  WIFI_CIPHER_TYPE_CCMP
I (2958) scan: Group Cipher     WIFI_CIPHER_TYPE_CCMP
I (2958) scan: Channel          7

I (2968) scan: SSID             华立语言村-办公
I (2968) scan: RSSI             -77
I (2978) scan: Authmode         WIFI_AUTH_WPA2_PSK
I (2978) scan: Pairwise Cipher  WIFI_CIPHER_TYPE_CCMP
I (2988) scan: Group Cipher     WIFI_CIPHER_TYPE_CCMP
I (2988) scan: Channel          6

I (2988) scan: SSID             xinyang-2.4G
I (2998) scan: RSSI             -78
I (2998) scan: Authmode         WIFI_AUTH_WPA_WPA2_PSK
I (3008) scan: Pairwise Cipher  WIFI_CIPHER_TYPE_CCMP
I (3008) scan: Group Cipher     WIFI_CIPHER_TYPE_CCMP
I (3018) scan: Channel          11

I (3018) scan: SSID             CU_Fjnp
I (3028) scan: RSSI             -88
I (3028) scan: Authmode         WIFI_AUTH_WPA_WPA2_PSK
I (3028) scan: Pairwise Cipher  WIFI_CIPHER_TYPE_CCMP
I (3038) scan: Group Cipher     WIFI_CIPHER_TYPE_TKIP
I (3048) scan: Channel          11

I (3048) scan: SSID             主色调童年美术馆
I (3048) scan: RSSI             -93
I (3058) scan: Authmode         WIFI_AUTH_WPA_WPA2_PSK
I (3058) scan: Pairwise Cipher  WIFI_CIPHER_TYPE_CCMP
I (3068) scan: Group Cipher     WIFI_CIPHER_TYPE_CCMP
I (3068) scan: Channel          11
```

## 三、注意事项

- `Kconfig.projbuild` 中对 `CONFIG_EXAMPLE_SCAN_LIST_SIZE`设定的范围为 0~20，默认值为10。如实际超出，需要手动更改。