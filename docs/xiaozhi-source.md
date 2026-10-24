# XiaoZhi 源码与官方镜像说明

## 项目中的源码副本

XiaoZhi ESP32 上游源码副本位于：

```text
vendor/xiaozhi-esp32
```

上游仓库：<https://github.com/78/xiaozhi-esp32.git>

当前源码版本提交为 `331d69b`。本开发板对应的板级目录：

```text
vendor/xiaozhi-esp32/main/boards/waveshare/esp32-s3-touch-lcd-1.83
```

主要文件为 `config.json`、`config.h`、`esp32-s3-touch-lcd-1.83.cc` 和 `README.md`。板型配置指定 Waveshare 厂商、ESP32-S3 目标、ST7789 屏幕、240×284 分辨率、ES8311/ES7210 音频芯片、AXP2101 电源管理芯片和 CST816 系列触摸驱动。

板级配置中，BOOT 键使用 `GPIO_NUM_0`，PWR 键使用 `GPIO_NUM_41`。当前 Brookesia 固件启用了触摸，但没有启用外部实体按键输入。

## Waveshare AIChats 源码集合

参考仓库：<https://github.com/waveshareteam/ESP32-AIChats>。该仓库收集不同版本的 Waveshare XiaoZhi 固件包，并列出 `ESP32-S3-Touch-LCD-1.83` 支持信息。曾短暂下载该仓库用于查找资料，因约 1.6GB 且只用于参考，之后已从 `vendor/` 移除。

## Waveshare 开发板示例

源码副本位于 `vendor/waveshare-demo`，上游仓库为 <https://github.com/waveshareteam/ESP32-S3-Touch-LCD-1.83.git>。它是开发板示例工程，包含 Arduino/ESP-IDF 示例和官方预编译恢复镜像，不是 XiaoZhi 应用源码。

## 官方恢复镜像

官方预编译镜像归档在：

```text
firmware/releases/waveshare-official-xiaozhi-20251024/ESP32-S3-Touch-LCD-1.83-xiaozhi-251024.bin
```

`esptool image-info` 检测到 ESP32-S3、16MB Flash、ESP-IDF `v5.5.1-dirty`，编译时间为 2025-10-21 11:21:12。该文件是固件，不是源码；官方没有标注构建它所用源码的确切提交，因此本地 `vendor/xiaozhi-esp32` 只能作为可编译和二次开发的源码基础，不能视为该镜像的逐字节对应源码。

镜像包含 `xiaozhi`、`XIAOZHI_STATE_EVENTS`、小智 OTA 地址和 Waveshare 板级类名等字符串，说明它很可能基于 XiaoZhi 或 Waveshare 对 XiaoZhi 的分支构建。同时镜像还包含 `esp_brookesia_phone`、`app_launcher`、`navigation_bar`、`recents_screen` 和 `status_bar` 等标记，表明官方整合镜像还包含多应用界面内容。

二进制记录的源码路径为 `./main/boards/waveshare-s3-touch-lcd-1.83/esp32-s3-touch-lcd-1.83.cc`，而当前上游副本的路径为 `main/boards/waveshare/esp32-s3-touch-lcd-1.83/esp32-s3-touch-lcd-1.83.cc`。因此不能认定官方镜像就是用当前本地源码版本构建的。

## 官方多应用固件源码线索

本地 `vendor/waveshare-demo/examples/ESP-IDF/03_esp-brookesia` 包含基于 Espressif Brookesia 的多应用界面示例，包括计算器、音乐、画板、简单电话和 Squareline 应用。相邻示例还包含重力水平仪、麦克风频谱分析仪和 TF 卡影音播放器。

官方恢复镜像同时具有 XiaoZhi 和 Brookesia 相关标记；公开资料中没有发现一个完整源码树能够与该整合镜像精确对应。应把它视为 Waveshare 发布的预编译集成固件，不能直接从单一公开源码目录还原。

## 二次开发方向

当前已归档的 XiaoZhi 2.2.6 烧录包是独立语音助手，Brookesia Claude 烧录包则是独立多应用状态显示固件。若要在小智助手中增加 Claude 状态显示，应在 XiaoZhi 源码上开发，例如增加 MCP 工具、串口命令监听或 LVGL 状态覆盖层，而不是直接修改 `.bin` 文件。
