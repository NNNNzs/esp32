# XiaoZhi 2.2.6 独立语音助手固件

适用开发板：Waveshare ESP32-S3-Touch-LCD-1.83。

## 固件类型

这是由项目内 `vendor/xiaozhi-esp32` 源码为本开发板编译的独立小智语音助手。它不是 Brookesia 多应用或设备管理固件，也不包含 Claude Code 状态显示。

## 构建信息

- 板型：`waveshare/esp32-s3-touch-lcd-1.83`
- 芯片：ESP32-S3，16MB Flash
- XiaoZhi：2.2.6
- ESP-IDF：5.5.4
- 分区表：`partitions-16m.csv`
- 应用镜像：`xiaozhi.bin`，2,828,256 字节
- 应用镜像 SHA-256：`3cc15b27b59942495928462bfbf40ed858cad22fc61c5e5dfa2d111fa45eabb2`

应用分区大小为 4032 KiB；应用镜像占用 `0x2B27E0` 字节，剩余空间为 31%。

## 文件说明

- `bootloader.bin`：二级引导程序，写入地址 `0x0`
- `partition-table.bin`：分区表，写入地址 `0x8000`
- `ota_data_initial.bin`：OTA 启动选择数据，写入地址 `0xD000`
- `xiaozhi.bin`：小智应用，写入地址 `0x20000`
- `generated_assets.bin`：界面和语音资源，写入地址 `0x800000`
- `partitions-16m.csv`：分区表源文件
- `flash_args`：匹配的烧录参数和地址
- `SHA256SUMS`：镜像、分区表和烧录参数校验和

## 烧录

在本目录执行，需使用含 esptool 的 Python 环境：

```sh
python3 -m esptool --chip esp32s3 -p /dev/cu.usbmodem1101 -b 460800 write_flash @flash_args
```

请将串口替换为设备当前端口。烧录命令会对镜像执行校验，但不会擦除或改写 `0x9000` 的 NVS 配置分区，因此设备已保存的 Wi-Fi 等设置会保留。
