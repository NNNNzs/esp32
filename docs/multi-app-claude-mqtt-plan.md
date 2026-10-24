# Brookesia 多应用与 Claude MQTT 方案

本方案说明如何以 Waveshare `03_esp-brookesia` 为基础，保留手机式 Wi-Fi 配网界面，增加 MQTT 驱动的 Claude Code 状态应用。小智 XiaoZhi 当前作为独立固件维护，尚未并入 Brookesia。

## 产品形态

固件基础：

```text
vendor/waveshare-demo/examples/ESP-IDF/03_esp-brookesia
```

目标体验：

- 开机进入手机式多应用启动器。
- 尽可能保留计算器、音乐和画板示例应用。
- 提供 `WiFi` 应用以查看网络状态和进入配网。
- 提供 `Claude` 应用，通过 MQTT 显示 Claude Code 状态。
- XiaoZhi 助手继续由单独固件提供；本方案不把小智固件当作 Brookesia 插件。

## 架构建议

Brookesia 作为多应用系统，小智作为独立语音助手固件。Brookesia 示例已有多应用壳，而 XiaoZhi 本身是完整应用/固件，两者都需要管理显示、LVGL、音频、Wi-Fi、NVS 和应用状态。

建议服务与应用模块：

```text
components/services/network_service
components/services/mqtt_status_service
components/apps/claude_status
components/apps/wifi_settings
```

### 网络服务

网络服务为各应用提供 Wi-Fi Station 模式和手机配网：

- 使用 ESP-IDF 原生 Wi-Fi 服务作为首版实现，源码位置可为 `components/claude_wifi_service`。
- NVS 中有 Wi-Fi 配置时，启动 Station 模式。
- 没有 Wi-Fi 配置时，启动 `ClaudeConfig-XXXX` 配网热点。
- 在 `http://192.168.4.1/` 提供配置页面，并将 Wi-Fi 和 MQTT 配置保存到 NVS。
- XiaoZhi 的 `vendor/xiaozhi-esp32/main/boards/common/wifi_board.cc` 可作为行为参考。

首版可复用 XiaoZhi 使用的 `78/esp-wifi-connect` 组件，但不要把 XiaoZhi 整个 `Application` 类搬入 Brookesia。应以 Brookesia 服务封装 Wi-Fi 事件，避免两套固件架构争夺网络和界面资源。

### MQTT 状态服务

MQTT 服务订阅 Claude Code 状态并通知 `Claude` 应用：

- 固件优先从 NVS 读取 Broker 和 Topic；无配置时回退到 `menuconfig` 默认值。
- Wi-Fi 配网页可编辑 Wi-Fi 与 MQTT 设置。
- PC Hook 脚本从环境变量读取 MQTT 设置。
- MQTT 客户端处理断线重连和退避。

建议 Topic：

```text
claudecode/device/<device_id>/status
```

建议消息格式：

```json
{
  "state": "thinking",
  "text": "可选的简短文本",
  "ts": 1780000000
}
```

状态值：

```text
thinking
approval
working
done
error
idle
```

PC 端 Claude Code Hook 通过 MQTT 发布状态，不直接写串口。

### Claude 状态应用

状态应用在屏幕上呈现当前状态：

- 使用小尺寸单色像素机器人图标，可用 LVGL 标签或位图资源实现。
- `thinking` 使用 3 至 4 帧动画。
- `approval` 显示静态提醒状态。
- `working` 显示扫描动画。
- `done` 显示已完成状态。
- 优先使用 32×32 或 48×48 索引色图标，并控制资源占用。

## XiaoZhi 与 Brookesia 的关系

当前两个固件分别构建和烧录。将 XiaoZhi 集成进 Brookesia 属于后续开发，不是现有小智固件的功能。

直接集成的主要难点：

- Brookesia 示例使用 LVGL 8.4.0，小智当前源码使用约 LVGL 9.5.0。
- 两边都需要管理显示、LVGL 主循环、音频编解码、Wi-Fi、NVS 和应用状态。
- 直接复制 XiaoZhi 代码进入 `components/apps` 容易出现版本冲突和维护困难。

若将来进行集成，可先在 Brookesia 启动器中增加仅用于配置/状态展示的 XiaoZhi 入口，再逐步移植网络协议、麦克风采集、扬声器播放、助手状态和 MCP 工具。直接把 Brookesia 升级到 LVGL 9 并移入小智内部模块的成本更高。

## 实施步骤

1. 保留官方整片镜像和各固件烧录包，进行主要固件试验前先备份设备 Flash。
2. 构建并验证 Brookesia 启动器、触摸屏和示例应用。
3. 增加 Wi-Fi 配网服务和 `WiFi` 应用，支持查看状态、进入配网、清除 Wi-Fi、编辑 MQTT Broker/Topic。
4. 增加 MQTT 客户端、NVS 配置和重连机制。
5. 增加 `ClaudeStatus` 应用，呈现状态图标与文本。
6. 将 PC Hook 脚本改为 MQTT 发布，并记录调试日志。
7. 在 MQTT 状态功能稳定后，再评估是否把小智助手功能移入 Brookesia。

## 主要风险

- Brookesia 与 XiaoZhi 的 LVGL 版本不一致。
- 两套固件同时管理音频编解码器或 Wi-Fi 时会发生资源冲突。
- 合并多应用、MQTT、语音和界面资源会增加 Flash、RAM 压力。
- 目前没有找到与 Waveshare 官方整合固件完全对应的公开完整源码树。

## 当前目标

Brookesia 的目标组合为：

```text
Brookesia 启动器 + 手机 Wi-Fi 配网 + MQTT Claude 状态应用
```

XiaoZhi 语音助手继续保留为独立固件，直到 Brookesia 状态功能稳定且集成方案明确。
