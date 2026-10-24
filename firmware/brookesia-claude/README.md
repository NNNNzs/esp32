# Brookesia Claude 固件

这是 Waveshare `ESP32-S3-Touch-LCD-1.83` 的项目主线固件，基于 Waveshare ESP-IDF 示例 `03_esp-brookesia` 修改。

不要直接修改 `vendor/waveshare-demo/examples/ESP-IDF/03_esp-brookesia` 中的原始副本。项目改动应放在本目录。

## 功能

- Brookesia 多应用启动器及计算器、音乐、画板和 Squareline 示例应用。
- `Claude` 应用：显示 Claude Code Hook 状态。
- `WiFi` 应用：查看设备 IP、配置网页和 MQTT 状态，并控制相关服务。
- Wi-Fi 与 MQTT 配置保存在 NVS 命名空间 `claude`。
- 没有已保存的 Wi-Fi 时，启动 WPA2 热点 `ClaudeConfig-XXXX` 和配置网页 `http://192.168.4.1/`。
- 已保存 Wi-Fi 时，以 Station 模式启动。
- MQTT 状态包括 `thinking`、`working`、`approval`、`done`、`error` 和 `idle`。

## 配置

优先读取 NVS；NVS 没有配置时使用 `menuconfig` 默认值：

```text
mqtt_broker -> CONFIG_CLAUDE_MQTT_BROKER_URI
mqtt_topic  -> CONFIG_CLAUDE_MQTT_TOPIC
wifi_ssid
wifi_pass
```

电脑端 Hook 发布脚本为 `tools/send_status_mqtt.py`，默认连接 `127.0.0.1:1883`，Topic 为 `claudecode/device/default/status`。可通过 `CLAUDE_MQTT_HOST`、`CLAUDE_MQTT_PORT`、`CLAUDE_MQTT_TOPIC` 和 `CLAUDE_MQTT_CLIENT_ID` 环境变量覆盖。

## 构建和烧录

本机已验证环境：ESP-IDF `C:\esp\v6.0.1\esp-idf`，目标 `esp32s3`。

在 Windows ESP-IDF PowerShell 中构建：

```powershell
py -3 ..\..\tools\build_firmware.py
```

指定串口烧录：

```powershell
py -3 ..\..\tools\build_firmware.py --flash --port COM3
```

将 `COM3` 换成设备当前串口。最近一次成功构建日期为 2026-05-28，应用镜像约 `0x35b950` 字节，最小应用分区为 `0x900000`。可复用的完整烧录包位于 `firmware/releases/brookesia-claude-20260528/`。

## 当前限制

- 保存 Wi-Fi/MQTT 配置后，建议重启开发板。
- MQTT 客户端随固件启动；若 Wi-Fi 尚未就绪，依赖 ESP-MQTT 自动重连。
- 配置网页启用时监听 80 端口，可在 `WiFi` 应用中停止。
- XiaoZhi 是单独的固件，没有集成进本 Brookesia 多应用固件。

更多构建与兼容说明见 [`../../docs/brookesia-claude-build.md`](../../docs/brookesia-claude-build.md)。
