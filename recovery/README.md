# Recovery

This folder keeps a known-good official firmware image and scripts for restoring or backing up the ESP32-S3-Touch-LCD-1.83 flash.

## Official Firmware

File:

```text
recovery/official-firmware/ESP32-S3-Touch-LCD-1.83-xiaozhi-251024.bin
```

Checked with `esptool image-info`:

- Image type: `ESP32-S3`
- Flash size: `16MB`
- Flash mode: `DIO`
- Flash frequency: `80m`
- ESP-IDF: `v5.5.1-dirty`
- Compile time: `Oct 21 2025 11:21:12`
- SHA256: `7B44F2BD49E0105C1657CADBD3773B5D73E1788B7661A3667C1279D0ACE24068`

The file is almost a full 16MB flash image, and its first bytes are an ESP32-S3 bootloader image. Restore it at address `0x0`.

## Restore Official Firmware

Set your port once:

```powershell
$env:ESP32_PORT = "COM3"
```

Restore:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\restore_official_firmware.ps1
```

Or pass the port explicitly:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\restore_official_firmware.ps1 -Port COM3
```

## Back Up Current Flash Before Experiments

This reads the entire 16MB flash into `recovery/backups`.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\backup_flash.ps1 -Port COM3
```

Important: the original factory system was already overwritten by the first Arduino upload. Future backups will protect whatever is currently on the device at the time you run the backup.
