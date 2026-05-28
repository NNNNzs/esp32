# Brookesia Claude Firmware

This is the current mainline firmware for the Waveshare `ESP32-S3-Touch-LCD-1.83`. It is based on Waveshare's ESP-IDF `03_esp-brookesia` multi-app demo.

Do not edit the original copy under `vendor/waveshare-demo/examples/ESP-IDF/03_esp-brookesia`. Project changes belong in this directory.

## Current Features

- Brookesia launcher plus demo apps: Calculator, Music, Draw, and Squareline.
- `Claude` status app for Claude Code hook state.
- `WiFi` settings app entry with live status for station IP, config web server, and MQTT client.
- Wi-Fi and MQTT config stored in NVS namespace `claude`.
- No saved Wi-Fi: starts WPA2 AP `ClaudeConfig-XXXX` and serves `http://192.168.4.1/`.
- Saved Wi-Fi: boots in station mode.
- `WiFi` app can start/stop the config web server and MQTT client from the device UI.
- MQTT status subscription for `thinking`, `working`, `approval`, `done`, `error`, and `idle`.

## Config

NVS values are preferred; `menuconfig` values are fallbacks:

```text
mqtt_broker -> CONFIG_CLAUDE_MQTT_BROKER_URI
mqtt_topic  -> CONFIG_CLAUDE_MQTT_TOPIC
wifi_ssid
wifi_pass
```

PC-side hook publisher:

```text
tools/send_status_mqtt.py
```

Script defaults:

```text
host: 127.0.0.1
port: 1883
topic: claudecode/device/default/status
```

Environment overrides:

```text
CLAUDE_MQTT_HOST
CLAUDE_MQTT_PORT
CLAUDE_MQTT_TOPIC
CLAUDE_MQTT_CLIENT_ID
```

## Build

Verified on this machine:

```text
ESP-IDF: C:\esp\v6.0.1\esp-idf
Target: esp32s3
```

Build:

```powershell
py -3 ..\..\tools\build_firmware.py
```

Latest 2026-05-27 build passed. App image is about `0x35b950`; smallest app partition is `0x900000`.

Flash:

```powershell
py -3 ..\..\tools\build_firmware.py --flash --port COM3
```

Replace `COM3` with the actual port.

## Known Limits

- After saving Wi-Fi/MQTT settings, the safest current flow is to restart the board.
- The MQTT client starts with the firmware; if Wi-Fi is not ready yet, it relies on ESP-MQTT reconnect.
- The config web page runs on port 80 when enabled and can be stopped from the `WiFi` app.
- XiaoZhi is not integrated into this Brookesia firmware yet.

More build and compatibility notes: `..\..\docs\brookesia-claude-build.md`.
