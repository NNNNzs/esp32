# Multi-App Claude MQTT Plan

Goal: build a multi-app firmware based on Waveshare `03_esp-brookesia`, keep phone-based Wi-Fi provisioning, add an MQTT-driven Claude Code status app, and later integrate XiaoZhi assistant behavior.

## Product Shape

Base firmware:

```text
vendor/waveshare-demo/examples/ESP-IDF/03_esp-brookesia
```

Target user experience:

- Device boots into a phone-like multi-app launcher.
- Existing demo apps still work where practical: calculator, music player, draw panel.
- A `WiFi` app can enter phone provisioning mode.
- A `Claude` app shows Claude Code status from MQTT.
- Later, a `XiaoZhi` app provides assistant behavior.

## Recommended Architecture

Use Brookesia as the main system, not XiaoZhi.

Reason:

- The user wants a multi-app launcher.
- `03_esp-brookesia` is already structured as a multi-app shell.
- XiaoZhi is closer to a full firmware/application than a small plug-in app.

Core services to add under `03_esp-brookesia`:

```text
components/services/network_service
components/services/mqtt_status_service
components/apps/claude_status
components/apps/wifi_settings
```

### Network Service

Purpose: provide Wi-Fi station mode and phone provisioning for all apps.

Current first implementation:

- Use a native ESP-IDF Wi-Fi service in `components/claude_wifi_service`.
- If saved Wi-Fi exists in NVS, boot as station.
- If no saved Wi-Fi exists, boot as an open config AP named `ClaudeConfig-XXXX`.
- The config page is served at `http://192.168.4.1/`.
- The same page saves Wi-Fi and MQTT config to NVS.

Best source to reuse:

```text
vendor/xiaozhi-esp32/main/boards/common/wifi_board.cc
```

Important XiaoZhi behavior to preserve:

- If saved Wi-Fi exists, connect automatically.
- If no saved Wi-Fi exists, start AP provisioning.
- AP SSID prefix is currently `Xiaozhi`.
- Phone connects to the AP and opens the provisioning web page.
- Manual reconfiguration calls `EnterWifiConfigMode()`.

Implementation preference:

- Reuse the same `78/esp-wifi-connect` component XiaoZhi uses.
- Do not import the whole XiaoZhi `Application` class into Brookesia for phase 1.
- Wrap Wi-Fi events in a small Brookesia-friendly service.
- Treat the native service as a bootstrap implementation that can be replaced by `esp-wifi-connect` after the ESP-IDF build environment is working.

### MQTT Status Service

Purpose: subscribe to Claude Code status messages and notify the Claude status app.

Configuration should be centralized:

- Firmware reads MQTT config from NVS first.
- If NVS has no value, firmware falls back to `menuconfig` defaults.
- The Wi-Fi/provisioning web page should eventually edit both Wi-Fi and MQTT settings.
- The PC hook script reads MQTT settings from environment variables.

Initial topic proposal:

```text
claudecode/device/<device_id>/status
```

Payload proposal:

```json
{
  "state": "thinking",
  "text": "optional short text",
  "ts": 1780000000
}
```

Supported states:

```text
thinking
approval
working
done
error
idle
```

PC-side Claude Code hooks should publish MQTT messages instead of writing to serial.

### Claude Status App

Purpose: visual status display.

Display style:

- Use a small monochrome/pixel-art robot face inspired by the Claude Code icon style.
- Store it as LVGL labels or bitmap assets.
- Avoid relying on serial.
- Animate `thinking` with 3-4 frames.

Example thinking animation idea:

```text
[ o_o ]  ->  [ o_O ]  ->  [ O_o ]  ->  [ o_o ]
```

Better final version:

- Use a 32x32 or 48x48 indexed bitmap icon.
- Add a tiny animated sparkle/dot orbit during `thinking`.
- Show `APPROVAL` as a still alert state.
- Show `WORKING` with a scanning line.
- Show `DONE` with a calm completed state.

## XiaoZhi App Integration Plan

Treat XiaoZhi as phase 2.

Reason:

- Brookesia demo uses LVGL 8.4.0.
- XiaoZhi currently depends on LVGL about 9.5.0.
- Both systems want ownership of display, LVGL loop, audio codec, Wi-Fi, NVS, and app state.
- Directly copying XiaoZhi into `components/apps` is likely to fail or become hard to maintain.

Practical route:

1. Keep Brookesia as the launcher.
2. Extract or recreate the Wi-Fi provisioning first.
3. Add Claude MQTT status app.
4. Add a `XiaoZhi` launcher app that initially shows setup/status.
5. Later port XiaoZhi features one by one:
   - network/protocol connection
   - microphone capture
   - speaker playback
   - assistant state display
   - MCP tools

Hard route:

- Upgrade Brookesia firmware to LVGL 9 and integrate XiaoZhi internals directly.
- This may require modifying Espressif Brookesia examples and all current apps.

## Development Phases

### Phase 0 - Recovery Safety

- Keep the official full flash image in `recovery/official-firmware`.
- Keep restore script working.
- Before testing each major firmware, run a fresh flash backup if the board contains anything valuable.

### Phase 1 - Brookesia Build Baseline

- Build `03_esp-brookesia` unchanged.
- Flash and verify display, touch, launcher, and bundled apps.
- Record exact ESP-IDF version and dependencies.

### Phase 2 - Wi-Fi Provisioning

- Add network service using the XiaoZhi-style `WifiManager`.
- Boot into provisioning if no Wi-Fi is saved.
- Reuse or extend the same phone web flow to save Claude MQTT settings.
- Add `WiFi` app with actions:
  - show current Wi-Fi status
  - enter provisioning mode
  - clear saved Wi-Fi
  - edit MQTT broker/topic

### Phase 3 - MQTT Transport

- Add MQTT client service.
- Store MQTT broker config in NVS.
- Subscribe to the Claude status topic.
- Add reconnect/backoff handling.

### Phase 4 - Claude Status App

- Add `ClaudeStatus` Brookesia app.
- Render status icon and text.
- Add thinking animation frames.
- Keep the latest status visible even if the app is not currently open, if Brookesia allows a notification/status mechanism.

### Phase 5 - PC Hook Publisher

- Replace serial hook script with MQTT publisher.
- Keep a log file for debugging.
- Publish Claude Code hook events:
  - prompt submitted -> `thinking`
  - pre tool use -> `working`
  - permission request -> `approval`
  - stop -> `done`
  - error path -> `error`

### Phase 6 - XiaoZhi Integration

- First integrate only the provisioning behavior and a XiaoZhi launcher tile.
- Then evaluate full assistant port after MQTT Claude status works.

## Main Risks

- LVGL version mismatch between Brookesia and XiaoZhi.
- Audio codec ownership conflicts.
- Wi-Fi manager conflicts if both systems initialize Wi-Fi independently.
- Flash/RAM pressure when combining Brookesia, MQTT, assets, and XiaoZhi.
- The official all-in-one firmware source does not appear to be published as one clean repository.

## First Implementation Target

The first working milestone should be:

```text
Brookesia launcher + phone Wi-Fi provisioning + MQTT Claude status app
```

Do not integrate full XiaoZhi assistant until this milestone is stable.
