# Brookesia Claude Firmware Build Notes

This file records the known-good build path for the multi-app Claude status firmware.

## Project

Firmware directory:

```text
firmware/brookesia-claude
```

This project is based on Waveshare's `03_esp-brookesia` example and adds:

- `Claude` status app
- `WiFi` settings app with live status and service controls
- Wi-Fi bootstrap/config storage services
- MQTT subscriber for Claude Code hook status
- HTTP config page for Wi-Fi and MQTT settings

## Build Environment

Known working environment on this machine:

```text
ESP-IDF: C:\esp\v6.0.1\esp-idf
Target: esp32s3
Board: Waveshare ESP32-S3-Touch-LCD-1.83
```

Build command:

```powershell
cd D:\project\esp32\firmware\brookesia-claude
. 'C:\esp\v6.0.1\esp-idf\export.ps1'
idf.py build
```

Successful build output:

```text
build\bootloader\bootloader.bin
build\partition_table\partition-table.bin
build\esp_brookesia_phone.bin
build\storage.bin
```

The app image from the latest successful build was about `0x35b950` bytes, and the smallest app partition was `0x900000`, so there was plenty of room left.

## Flash Warning

Flashing this firmware replaces the currently installed firmware. Before flashing, make sure the official firmware recovery file is still present:

```text
recovery\official-firmware\ESP32-S3-Touch-LCD-1.83-xiaozhi-251024.bin
```

Restore command if needed:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\restore_official_firmware.ps1 -Port COM3
```

## ESP-IDF 6 Compatibility Patches

ESP-IDF 6 split several old `driver/...` headers into separate components. Some Waveshare/managed components did not declare those dependencies yet, so this project has local compatibility edits in `managed_components`:

- `managed_components/espressif__esp_codec_dev/CMakeLists.txt`
  - added `esp_driver_gpio`, `esp_driver_i2c`, `esp_driver_i2s`, `esp_driver_spi`
- `managed_components/espressif__esp_codec_dev/platform/audio_codec_ctrl_spi.c`
  - added `#include "esp_idf_version.h"`
- `managed_components/chmorgan__esp-audio-player/CMakeLists.txt`
  - added `esp_driver_i2s`
- `managed_components/chmorgan__esp-audio-player/audio_player.cpp`
  - removed a C++ warning-triggering cast around `NULL`
- `managed_components/waveshare__esp32_s3_touch_lcd_1_83/CMakeLists.txt`
  - added split driver dependencies such as `esp_driver_i2s`, `esp_driver_ledc`, `esp_driver_sdmmc`
- `managed_components/waveshare__esp32_s3_touch_lcd_1_83/include/bsp/display.h`
  - changed LCD color order macro to `LCD_RGB_ELEMENT_ORDER_RGB`
- `managed_components/waveshare__esp32_s3_touch_lcd_1_83/esp32_s3_touch_lcd_1_83.c`
  - fixed `bsp_display_lcd_init()` call signature

Project-local CMake edits were also needed:

- `main/CMakeLists.txt`
  - explicitly depends on apps, BSP, Brookesia, LVGL port, NVS, UART/GPIO, MQTT/Wi-Fi/config services
- `components/apps/CMakeLists.txt`
  - applies C-only warning flags only to C files
- `components/bsp_extra/CMakeLists.txt`
  - declares split driver dependencies

These `managed_components` edits can be overwritten if dependencies are deleted and re-resolved. If that happens, reapply the notes above or move the patched components into a persistent local component override.

## Flash Test

The ESP-IDF build was flashed successfully to `COM3` on 2026-05-27.

Flash command:

```powershell
cd D:\project\esp32\firmware\brookesia-claude
. 'C:\esp\v6.0.1\esp-idf\export.ps1'
idf.py -p COM3 flash
```

Serial boot log confirmed:

- ESP32-S3 detected with 16MB flash and 8MB PSRAM
- SPIFFS mounted successfully
- audio codec initialized
- display and touch initialized
- Brookesia launcher started
- no boot crash or reset loop observed

Because no Wi-Fi credentials were stored yet, the firmware started the config hotspot:

```text
SSID: ClaudeConfig-5675
Config URL: http://192.168.4.1/
```

The exact SSID suffix is based on the board MAC and may change if using another board.
The hotspot is WPA2-protected, and the `WiFi` app now reports the station IP, config web server state, and MQTT client state.

## Current Limitations

- The config web page stores Wi-Fi and MQTT settings in NVS, but after saving Wi-Fi settings the simplest expected flow is to restart the board.
- The config web page can be stopped and restarted from the `WiFi` app.
- XiaoZhi has not been integrated into this firmware yet; this is the Brookesia multi-app base with Claude status/MQTT added.
