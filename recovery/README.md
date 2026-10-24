# 固件恢复与设备备份

本目录存放 Waveshare ESP32-S3-Touch-LCD-1.83 的恢复和整片 Flash 备份脚本。官方镜像与项目编译的烧录包统一保存在 `firmware/releases/`。

## 官方恢复镜像

文件：

```text
firmware/releases/waveshare-official-xiaozhi-20251024/ESP32-S3-Touch-LCD-1.83-xiaozhi-251024.bin
```

`esptool image-info` 检测结果：

- 芯片：ESP32-S3
- Flash：16MB
- 模式：DIO
- 频率：80MHz
- ESP-IDF：`v5.5.1-dirty`
- 编译时间：2025-10-21 11:21:12
- SHA-256：`7b44f2bd49e0105c1657cadbd3773b5d73e1788b7661a3667c1279d0ace24068`

该文件接近完整 16MB Flash 镜像，首段为 ESP32-S3 Bootloader。恢复时从地址 `0x0` 写入。

## 恢复官方固件

可先设置串口：

```powershell
$env:ESP32_PORT = "COM3"
```

恢复：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\restore_official_firmware.ps1
```

也可直接指定串口：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\restore_official_firmware.ps1 -Port COM3
```

## 备份当前 Flash

备份脚本读取整片 16MB Flash 并保存到 `recovery/backups/`：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\backup_flash.ps1 -Port COM3
```

开发板原始出厂系统此前已被覆盖。此后备份保存的是备份当时设备上实际运行的固件和数据。
