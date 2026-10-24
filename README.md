# Waveshare ESP32-S3-Touch-LCD-1.83 固件项目

本项目为 Waveshare `ESP32-S3-Touch-LCD-1.83` 开发板保存三条固件线：官方恢复镜像、小智 XiaoZhi 独立语音助手，以及本项目自定义的 Brookesia Claude 多应用固件。

## 固件清单

| 固件 | 源码或来源 | 烧录包 | 用途 |
| --- | --- | --- | --- |
| Waveshare 官方固件 | 官方提供的预编译镜像，没有随镜像发布的完整源码版本 | `firmware/releases/waveshare-official-xiaozhi-20251024/` | 恢复官方提供的固件镜像 |
| XiaoZhi 2.2.6 | `vendor/xiaozhi-esp32/` | `firmware/releases/xiaozhi-2.2.6-waveshare-esp32-s3-touch-lcd-1.83-20261024/` | 独立的小智语音助手 |
| Brookesia Claude | `firmware/brookesia-claude/` | `firmware/releases/brookesia-claude-20260528/` | 多应用启动器与 Claude Code 状态显示 |

三份烧录包集中在 [`firmware/releases/README.md`](firmware/releases/README.md)，设备备份、固件来源和当前运行状态见[固件清单](docs/firmware-inventory.md)。

## 目录结构

```text
firmware/
├── brookesia-claude/       Brookesia Claude 固件源码
└── releases/               三条固件线的烧录包、说明和校验和
    ├── waveshare-official-xiaozhi-20251024/
    ├── xiaozhi-2.2.6-waveshare-esp32-s3-touch-lcd-1.83-20261024/
    └── brookesia-claude-20260528/
vendor/
├── xiaozhi-esp32/          XiaoZhi 上游源码副本，供编译和参考
└── waveshare-demo/         Waveshare 官方示例源码，供硬件和驱动参考
recovery/                   官方镜像恢复脚本和整片 Flash 备份脚本
tools/                      构建、烧录和 Claude Code 状态发布工具
docs/                       固件清单、构建记录和源码说明
```

`vendor/` 是常见的项目目录名称，用于存放外部项目的源码副本或第三方依赖。这里的 `xiaozhi-esp32` 和 `waveshare-demo` 是源码与示例，不是烧录包；实际可烧录文件统一放在 `firmware/releases/`。Waveshare 官方源码仓库与 XiaoZhi 上游源码也不是同一个项目。

## 当前项目主线

项目自行开发的主线是 `firmware/brookesia-claude/`，它基于 Waveshare 的 ESP-IDF Brookesia 多应用示例，增加 Wi-Fi 设置、MQTT 状态接收和 Claude Code 状态显示。小智固件是单独的语音助手程序，不包含 Brookesia 多应用管理界面或 Claude 状态显示。

Brookesia 固件最近一次记录的构建环境：

- ESP-IDF：6.0.1
- 芯片：`esp32s3`
- 通过构建验证：2026-05-28

## 构建 Brookesia 固件

在 Windows 的 ESP-IDF PowerShell 环境中运行：

```powershell
py -3 .\tools\build_firmware.py
```

构建并烧录到指定串口：

```powershell
py -3 .\tools\build_firmware.py --flash --port COM3
```

将 `COM3` 替换为开发板当前串口。已归档的固件包可按各自目录的 README 直接烧录，不需要重新编译。

## Brookesia 设备配置

首次启动且没有已保存的 Wi-Fi 配置时，Brookesia 固件会启动 WPA2 热点：

```text
ClaudeConfig-XXXX
```

连接热点后访问 `http://192.168.4.1/`，填写 Wi-Fi、MQTT Broker 和 Topic。保存后重启开发板。`WiFi` 应用可查看设备 IP、配置网页和 MQTT 客户端状态。

默认 MQTT 设置：

```text
Broker: mqtt://192.168.1.2:1883
Topic:  claudecode/device/default/status
```

## Claude Code Hook

项目 Hook 配置位于 `.claude/settings.json`，默认状态发布脚本为 `tools/send_status_mqtt.py`。可通过以下环境变量覆盖 MQTT 配置：

```powershell
$env:CLAUDE_MQTT_HOST="127.0.0.1"
$env:CLAUDE_MQTT_PORT="1883"
$env:CLAUDE_MQTT_TOPIC="claudecode/device/default/status"
```

事件与状态对应关系：

```text
UserPromptSubmit  -> thinking
PermissionRequest -> approval
PreToolUse        -> working
Stop              -> done
```

Hook 调试日志：

```powershell
Get-Content .\.claude\hook-status.log -Tail 20
```

## 官方固件恢复与设备备份

恢复官方预编译镜像：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\restore_official_firmware.ps1 -Port COM3
```

备份当前 16MB Flash：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\backup_flash.ps1 -Port COM3
```

恢复操作会替换设备当前固件。执行前请阅读 [`recovery/README.md`](recovery/README.md)，并确认官方镜像仍在 `firmware/releases/`。

## 参考源码

- Waveshare 开发板示例：`vendor/waveshare-demo`
- XiaoZhi ESP32 上游源码：`vendor/xiaozhi-esp32`
- XiaoZhi 对应本开发板的配置：`vendor/xiaozhi-esp32/main/boards/waveshare/esp32-s3-touch-lcd-1.83`

Waveshare 开发板两个按键在 XiaoZhi 板级配置中的 GPIO 分别是 `GPIO_NUM_0` 和 `GPIO_NUM_41`。当前 Brookesia BSP 未启用外部按键输入。

重新生成被删除的离线 Wiki 和固件资源：

```powershell
py -3 .\tools\rebuild_generated.py
```
