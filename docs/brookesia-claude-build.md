# Brookesia Claude 固件构建记录

本文记录 Brookesia 多应用 Claude 状态固件已验证的构建和烧录环境。

## 工程内容

源码目录：

```text
firmware/brookesia-claude/
```

工程基于 Waveshare `03_esp-brookesia` 示例，增加了 Claude 状态应用、带实时状态和服务控制的 Wi-Fi 设置应用、Wi-Fi 配置服务、MQTT 状态订阅服务，以及用于配置 Wi-Fi 和 MQTT 的 HTTP 页面。

## 构建环境

本机已验证环境：

```text
ESP-IDF: C:\esp\v6.0.1\esp-idf
目标芯片: esp32s3
开发板: Waveshare ESP32-S3-Touch-LCD-1.83
```

构建命令：

```powershell
cd D:\project\esp32\firmware\brookesia-claude
. 'C:\esp\v6.0.1\esp-idf\export.ps1'
idf.py build
```

成功构建会生成：

```text
build\bootloader\bootloader.bin
build\partition_table\partition-table.bin
build\esp_brookesia_phone.bin
build\storage.bin
```

最近一次成功构建的应用镜像约 `0x35b950` 字节，最小应用分区为 `0x900000`。归档烧录包位于 `firmware/releases/brookesia-claude-20260528/`。

## 烧录前须知

烧录 Brookesia 会替换设备当前固件。恢复用官方镜像应保存在：

```text
firmware\releases\waveshare-official-xiaozhi-20251024\ESP32-S3-Touch-LCD-1.83-xiaozhi-251024.bin
```

需要恢复时运行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\restore_official_firmware.ps1 -Port COM3
```

## ESP-IDF 6 兼容补丁

ESP-IDF 6 将多个旧版 `driver/...` 头文件拆分为独立组件，而部分 Waveshare 或托管组件尚未声明这些依赖。构建时使用了以下本地兼容修改：

- `managed_components/espressif__esp_codec_dev/CMakeLists.txt`：增加 GPIO、I2C、I2S 和 SPI 驱动依赖。
- `managed_components/espressif__esp_codec_dev/platform/audio_codec_ctrl_spi.c`：增加 `esp_idf_version.h`。
- `managed_components/chmorgan__esp-audio-player/CMakeLists.txt`：增加 I2S 驱动依赖。
- `managed_components/chmorgan__esp-audio-player/audio_player.cpp`：移除触发 C++ 警告的 `NULL` 转换。
- `managed_components/waveshare__esp32_s3_touch_lcd_1_83/CMakeLists.txt`：增加 I2S、LEDC、SDMMC 等拆分驱动依赖。
- `managed_components/waveshare__esp32_s3_touch_lcd_1_83/include/bsp/display.h`：将 LCD 颜色顺序宏改为 `LCD_RGB_ELEMENT_ORDER_RGB`。
- `managed_components/waveshare__esp32_s3_touch_lcd_1_83/esp32_s3_touch_lcd_1_83.c`：修复 `bsp_display_lcd_init()` 调用签名。
- `main/CMakeLists.txt`：明确声明应用、BSP、Brookesia、LVGL、NVS、UART/GPIO、MQTT、Wi-Fi 和配置服务依赖。
- `components/apps/CMakeLists.txt`：将仅适用于 C 的警告参数限制在 C 文件。
- `components/bsp_extra/CMakeLists.txt`：声明拆分后的驱动依赖。

删除并重新解析 `managed_components` 后，这些本地改动可能被覆盖。届时需要重新应用，或将补丁组件迁移到持久的本地组件覆盖目录。

## 已验证烧录记录

2026-05-27 曾通过 `COM3` 成功烧录并启动。串口日志确认识别到 ESP32-S3、16MB Flash 和 8MB PSRAM，SPIFFS、音频编解码、屏幕和触摸均初始化，随后进入 Brookesia 启动器，未观察到启动崩溃或复位循环。

当时设备没有 Wi-Fi 凭据，因此启动了 WPA2 配网热点 `ClaudeConfig-5675`，配置地址为 `http://192.168.4.1/`。SSID 后缀根据设备 MAC 生成，其他设备可能不同。

## 当前限制

- Wi-Fi/MQTT 网页配置写入 NVS；保存后建议重启开发板。
- 配置网页可在 `WiFi` 应用中停止或重新启动。
- XiaoZhi 没有集成到该固件；此固件是带 Claude 状态和 MQTT 功能的 Brookesia 多应用版本。
