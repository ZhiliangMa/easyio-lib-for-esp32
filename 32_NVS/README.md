# 32_NVS

## 例程简介

使用 `FreeRTOS` 的 `Task`，创建两个任务：

1. **led_task** ，控制LED闪烁

2. **nvs_storage_task** ，初始化 `NVS`，来记录总开机上电次数。

主要用于在 flash 中存储`键值`格式的数据。很适合用来存储`系统设置`之类的变量。

乐鑫官方的NVS文档：[ESP32 - NVS文档](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/storage/nvs_flash.html)


## NVS操作的存储对象：

### 键值对：

NVS 的操作对象为键值对，其中键是 `ASCII 字符串`，当前支持最大键长为 15 个字符，值可以为以下几种类型：

* 整数型：`uint8_t`、`int8_t`、`uint16_t`、`int16_t`、`uint32_t`、`int32_t`、`uint64_t` 和 `int64_t`；

* 以 `\0` 结尾的字符串；

* 可变长度的二进制数据 (BLOB)

`键必须唯一`。为现有的键写入新的值可能产生如下结果：

* 如果新旧值数据类型相同，则更新值；

* 如果新旧值数据类型不同，则返回错误。

### 命名空间：

为了减少不同组件之间键名的潜在冲突，NVS 将一组键值对分配给一个命名空间。命名空间的命名规则遵循键名的命名规则，即最多可占 15 个字符。命名空间的名称在调用 `nvs_open` 或 `nvs_open_from_part` 中指定，调用后将返回一个不透明句柄，用于后续调用 `nvs_get_*`、`nvs_set_*` 和 `nvs_commit` 函数。这样，一个句柄关联一个命名空间，键名便不会与其他命名空间中相同键名冲突。请注意，不同 NVS 分区中具有相同名称的命名空间将被视为不同的命名空间。
（简单来说就是，不同应用，使用不同的命名空间，以避免不同组件之间键名的冲突。命名空间相当于文件夹，键值对相当于其中的文件）


## Demo运行现象

* LED闪烁。

* 打开终端，ESP32上电运行后会打印烧录代码后的`总上电次数`。

* 保持连接终端，手动按ESP32的复位按键，随着`总上电次数`（复位次数）增加，会看到 `Restart counter`递增。


## 关键函数

见乐鑫官方的NVS文档：[ESP32 - NVS文档](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/api-reference/storage/nvs_flash.html)


## 注意事项

* NVS只能存储`键值对`，只适合存储`系统设置`或者其他信息不多的应用。
