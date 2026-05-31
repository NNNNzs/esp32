# XiaoZhi Source Notes

This project now keeps local copies of the likely source code for the official XiaoZhi firmware path.

## What We Found

### Upstream XiaoZhi ESP32 Source

Local path:

```text
vendor/xiaozhi-esp32
```

Remote:

```text
https://github.com/78/xiaozhi-esp32.git
```

Current local commit:

```text
331d69b
```

Important board path for this device:

```text
vendor/xiaozhi-esp32/main/boards/waveshare/esp32-s3-touch-lcd-1.83
```

Key files:

```text
config.json
config.h
esp32-s3-touch-lcd-1.83.cc
README.md
```

This board entry sets:

- Manufacturer: `waveshare`
- Target: `esp32s3`
- Build name: `esp32-s3-touch-lcd-1.83`
- Display: ST7789 over SPI
- Display size: `240 x 284`
- Audio: ES8311 + ES7210
- PMIC: AXP2101
- Touch: CST816S/CST816 family driver over I2C

Physical button mapping found in the board config:

- `BOOT_BUTTON_GPIO` -> `GPIO_NUM_0`
- `PWR_BUTTON_GPIO` -> `GPIO_NUM_41`

Current firmware note:

- The Brookesia firmware in this repository currently initializes touch input, but it does not expose the board's external buttons as active UI inputs.
- The BSP header for `firmware/brookesia-claude` also reports `BSP_CAPS_BUTTONS 0`, so the two right-side buttons are effectively unused by the current firmware build.

### Waveshare AIChats Collection

Online reference:

```text
https://github.com/waveshareteam/ESP32-AIChats
```

This repository collects Waveshare-specific XiaoZhi packages by version and lists `ESP32-S3-Touch-LCD-1.83` as supported.

It was briefly cloned during source discovery, but removed from `vendor/` because the checkout was about 1.6GB and was only needed as a reference.

### Waveshare Board Demo Source

Local path:

```text
vendor/waveshare-demo
```

Remote:

```text
https://github.com/waveshareteam/ESP32-S3-Touch-LCD-1.83.git
```

This is the board sample/demo repository, not the XiaoZhi application source. It contains Arduino and ESP-IDF demos plus the prebuilt official firmware image.

## Relationship To The Official Firmware Bin

The official recovery image we saved is:

```text
recovery/official-firmware/ESP32-S3-Touch-LCD-1.83-xiaozhi-251024.bin
```

This is compiled firmware, not source code. `esptool image-info` reports:

- ESP32-S3 image
- 16MB flash image
- ESP-IDF `v5.5.1-dirty`
- Compile time `Oct 21 2025 11:21:12`

The most likely editable source base is `vendor/xiaozhi-esp32`, using the board support under:

```text
main/boards/waveshare/esp32-s3-touch-lcd-1.83
```

The exact source commit used by Waveshare for `ESP32-S3-Touch-LCD-1.83-xiaozhi-251024.bin` is not stated in the firmware filename or local docs. Treat the local upstream clone as the current source base, not a byte-for-byte match to that firmware.

## Is The Official Firmware Built From XiaoZhi?

Short answer: very likely yes.

Evidence extracted from the official firmware image:

- The firmware filename contains `xiaozhi`.
- `esptool image-info` detects an ESP32-S3 image with ESP-IDF `v5.5.1-dirty`, matching the kind of ESP-IDF project XiaoZhi uses.
- The binary contains strings such as `xiaozhi`, `Xiaozhi`, `XIAOZHI_STATE_EVENTS`, and `https://api.tenclass.net/xiaozhi/ota/`.
- The binary contains the MCP initialization fragment:

```text
"type":"mcp"
"serverInfo":{"name":"waveshare-s3-touch-lcd-1.83"
```

- The binary contains the board class/tag string:

```text
WaveshareEsp32s3TouchLCD1inch83
```

which also exists in:

```text
vendor/xiaozhi-esp32/main/boards/waveshare/esp32-s3-touch-lcd-1.83/esp32-s3-touch-lcd-1.83.cc
```

- The binary contains matching board initialization logs and source symbols, including:

```text
Init AXP2101
Initialize touch controller
Touch panel initialized successfully
esp_lcd_new_panel_st7789
esp_lcd_touch_new_i2c_cst816s
ES8311
ES7210
```

These line up with the current XiaoZhi board source for this device.

Important caveat: the binary contains a source path like:

```text
./main/boards/waveshare-s3-touch-lcd-1.83/esp32-s3-touch-lcd-1.83.cc
```

while the currently cloned upstream source uses:

```text
main/boards/waveshare/esp32-s3-touch-lcd-1.83/esp32-s3-touch-lcd-1.83.cc
```

So the official firmware was almost certainly built from the XiaoZhi codebase or a close Waveshare fork, but not necessarily from the exact current checkout in `vendor/xiaozhi-esp32`.

## Official Multi-App Firmware Search

The factory experience appears to be more than the upstream XiaoZhi app alone.

What is available locally:

- `vendor/xiaozhi-esp32` contains the XiaoZhi application and board support for this device.
- `vendor/waveshare-demo/examples/ESP-IDF/03_esp-brookesia` contains a phone-like multi-app shell using Espressif `esp-brookesia`.
- `03_esp-brookesia/main/main.cpp` installs these apps into the shell:
  - `Calculator`
  - `MusicPlayer`
  - `Drawpanel`
  - `PhoneAppSimpleConf`
  - `PhoneAppSquareline`
- `vendor/waveshare-demo/examples/ESP-IDF/04_Immersive_block` is the QMI8658 gravity/level-style block demo.
- `vendor/waveshare-demo/examples/ESP-IDF/05_Spec_Analyzer` is the microphone audio spectrum analyzer demo.
- `vendor/waveshare-demo/examples/ESP-IDF/06_videoplayer` is the TF-card AVI video/audio player demo.

What the official documentation says:

- The Waveshare wiki lists ESP-IDF demos `03_esp-brookesia`, `04_Immersive_block`, `05_Spec_Analyzer`, and `06_videoplayer`.
- The same page says the `Firmware` directory contains prebuilt test firmware for direct flashing.
- The XiaoZhi upstream release notes mention `ESP32-S3-Touch-LCD-1.83` as a newly added third-party board, but do not describe Waveshare's multi-app test shell.

What the official recovery binary suggests:

- The binary contains XiaoZhi strings and the matching Waveshare XiaoZhi board class.
- The same binary also contains `esp_brookesia_phone`, `app_launcher`, `navigation_bar`, `recents_screen`, and `status_bar` strings.

Conclusion:

The closest public sources are split across two places: XiaoZhi upstream for the assistant app, and Waveshare's ESP-IDF demos for the multi-app UI and hardware tests. I did not find a single public source tree that exactly corresponds to the all-in-one factory firmware image `ESP32-S3-Touch-LCD-1.83-xiaozhi-251024.bin`. Treat that image as a Waveshare-built integration or fork, not as something we can edit directly from one complete released source package.

## Next Development Direction

To keep the official XiaoZhi system and add Claude Code status display behavior, modify the XiaoZhi source rather than patching the `.bin`.

Candidate integration points:

- Add a new MCP tool in `esp32-s3-touch-lcd-1.83.cc`, similar to `self.system.reconfigure_wifi`.
- Add a serial command listener that changes the LVGL display state.
- Add a small overlay/status component in the display layer.

The cleanest first experiment is adding a new MCP or serial command that displays `thinking`, `approval`, `working`, and `done` without replacing the whole XiaoZhi UI.
