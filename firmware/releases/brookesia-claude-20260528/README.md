# Brookesia Claude 多应用固件

适用开发板：Waveshare ESP32-S3-Touch-LCD-1.83。

## 固件类型

本项目自行开发的 Brookesia 固件。源码位于 `firmware/brookesia-claude/`，提供 Brookesia 多应用启动器和 Claude Code 状态显示；它不是独立的小智语音助手。

## 构建信息

- 芯片：ESP32-S3，16MB Flash
- ESP-IDF：6.0.1
- 构建日期：2026-05-28
- 分区表：`partitions.csv`
- 应用分区：`0x10000`，9MB
- 存储分区：`0x910000`，4MB
- 应用镜像 SHA-256：`120938417735568354c96e102c3f37456b944d355e0e33b0fc93e29442e6da52`

## 文件说明

- `bootloader.bin`：二级引导程序，写入地址 `0x0`
- `partition-table.bin`：分区表，写入地址 `0x8000`
- `esp_brookesia_phone.bin`：Brookesia 应用，写入地址 `0x10000`
- `storage.bin`：生成的存储镜像，写入地址 `0x910000`
- `partitions.csv`：分区表源文件
- `flash_args`：对应的烧录参数和地址
- `SHA256SUMS`：镜像和分区文件校验和

## 烧录

在本目录执行，需使用已安装 esptool 的 ESP-IDF Python 环境：

```sh
python3 -m esptool --chip esp32s3 -p /dev/cu.usbmodem1101 -b 460800 write_flash @flash_args
```

请将串口替换为设备当前端口。该命令写入 bootloader、分区表、应用和存储镜像，不覆盖 NVS 配置分区。
