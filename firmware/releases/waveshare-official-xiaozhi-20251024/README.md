# Waveshare 官方恢复固件

适用开发板：Waveshare ESP32-S3-Touch-LCD-1.83。

这是 Waveshare 发布的预编译恢复镜像，不是本项目从源码构建的产物。官方仓库没有说明该镜像所用源码的确切提交版本。

- 芯片：ESP32-S3，16MB Flash
- `esptool image-info` 检测到的 ESP-IDF 版本：`v5.5.1-dirty`
- 编译时间：2025-10-21 11:21:12
- 文件大小：16,732,160 字节
- 烧录地址：`0x0`
- SHA-256：`7b44f2bd49e0105c1657cadbd3773b5d73e1788b7661a3667c1279d0ace24068`

使用项目恢复脚本：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\recovery\restore_official_firmware.ps1 -Port COM3
```
