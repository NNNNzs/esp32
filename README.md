# ESP32-S3 Claude Code Status Display

Target board: Waveshare `ESP32-S3-Touch-LCD-1.83`.

Current mainline firmware is the ESP-IDF/Brookesia multi-app build in `firmware/brookesia-claude`. It receives Claude Code hook state over MQTT and shows it on the LCD.

## Current Status

- Firmware: `firmware/brookesia-claude`
- ESP-IDF: `C:\esp\v6.0.1\esp-idf`
- Target: `esp32s3`
- Build: verified on 2026-05-28 with `idf.py build`
- Board test: flashed to `COM3`; serial log showed the Brookesia launcher starting normally
- First boot without saved Wi-Fi: starts `ClaudeConfig-XXXX` with WPA2 and serves setup at `http://192.168.4.1/`
- Wi-Fi settings app shows station IP, config web status, and MQTT status; it can start/stop the config web server and MQTT client

## Layout

```text
firmware/brookesia-claude/   Current ESP-IDF firmware
claudecode_status_display/   Early Arduino serial status sketch, kept as fallback
tools/                       PC-side status sender scripts
recovery/                    Official firmware restore and flash backup scripts
docs/                        Project notes and offline material index
vendor/                      Waveshare and XiaoZhi reference source
```

## Build

```powershell
py -3 .\tools\build_firmware.py
```

Flash:

```powershell
py -3 .\tools\build_firmware.py --flash --port COM3
```

Replace `COM3` with the actual port.

## Device Setup

If no Wi-Fi credentials are saved, the firmware starts a WPA2 setup AP:

```text
ClaudeConfig-XXXX
```

Connect to it and open:

```text
http://192.168.4.1/
```

Enter Wi-Fi SSID/password, MQTT broker URI, and topic. After saving, restart the board so it can connect in station mode.
The `WiFi` app also shows the current station IP, whether the config web server is running, and whether the MQTT client is running.

Firmware MQTT defaults:

```text
broker: mqtt://192.168.1.2:1883
topic:  claudecode/device/default/status
```

## Claude Code Hook

Project hook config:

```text
.claude/settings.json
```

Default PC-side publisher:

```text
tools/send_status_mqtt.py
```

Environment overrides:

```powershell
$env:CLAUDE_MQTT_HOST="127.0.0.1"
$env:CLAUDE_MQTT_PORT="1883"
$env:CLAUDE_MQTT_TOPIC="claudecode/device/default/status"
```

Hook state mapping:

```text
UserPromptSubmit  -> thinking
PermissionRequest -> approval
PreToolUse        -> working
Stop              -> done
```

Debug log:

```powershell
Get-Content .\.claude\hook-status.log -Tail 20
```

If the log changes but the screen does not, check the MQTT broker, PC LAN IP, device Wi-Fi, and topic.

## Recovery

Official firmware image:

```text
recovery/official-firmware/ESP32-S3-Touch-LCD-1.83-xiaozhi-251024.bin
```

Restore:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\restore_official_firmware.ps1 -Port COM3
```

Back up current flash:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\backup_flash.ps1 -Port COM3
```

Restore writes the official image at `0x0` and replaces the current firmware. Read `recovery/README.md` first.

## Reference Source

- Waveshare demo: `vendor/waveshare-demo`
- XiaoZhi source: `vendor/xiaozhi-esp32`
- Waveshare XiaoZhi board directory: `vendor/xiaozhi-esp32/main/boards/waveshare/esp32-s3-touch-lcd-1.83`

XiaoZhi is not integrated into the Brookesia firmware yet. It is currently reference material for later provisioning and assistant work.

## Generated Content

To rebuild generated assets after deleting them, use:

```powershell
py -3 .\tools\rebuild_generated.py
```

That script regenerates the offline wiki mirror and rebuilds the firmware.
