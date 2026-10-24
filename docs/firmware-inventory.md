# 固件清单

本文统一管理 Waveshare ESP32-S3-Touch-LCD-1.83 项目的三条固件线，并单独记录设备闪存备份和回读文件。

## 三条固件线

| # | 名称 | 位置 | 类型与状态 | 能否直接烧录 |
| --- | --- | --- | --- | --- |
| 1 | Waveshare 官方恢复固件 | `firmware/releases/waveshare-official-xiaozhi-20251024/` | 官方仓库提供的预编译整片恢复镜像；不是本项目自行编译，也没有对应的官方完整源码提交信息。文件大小 16,732,160 字节，SHA-256：`7b44f2bd49e0105c1657cadbd3773b5d73e1788b7661a3667c1279d0ace24068`。`esptool image-info` 识别为 ESP32-S3、16MB Flash、DIO、80MHz、ESP-IDF `v5.5.1-dirty`，编译时间 `2025-10-21 11:21:12`。 | 可以。恢复脚本从地址 `0x0` 写入。 |
| 2 | XiaoZhi 独立语音助手 | 源码：`vendor/xiaozhi-esp32/`；烧录包：`firmware/releases/xiaozhi-2.2.6-waveshare-esp32-s3-touch-lcd-1.83-20261024/` | XiaoZhi 2.2.6，ESP-IDF 5.5.4，目标板 Waveshare 1.83。烧录包包含 bootloader、分区表、OTA 初始数据、应用镜像和资源镜像。它是独立的小智语音助手，不是 Brookesia 多应用/设备管理固件，也不含 Claude 状态显示。 | 可以。按同目录 `README.md` 中的 `flash_args` 使用整套匹配文件烧录。 |
| 3 | Brookesia Claude 多应用固件 | 源码：`firmware/brookesia-claude/`；烧录包：`firmware/releases/brookesia-claude-20260528/` | 本项目自定义 Brookesia 多应用固件，含 Claude Code 状态显示。ESP-IDF 6.0.1，构建日期 2026-05-28。烧录包包含 bootloader、分区表、应用镜像和 storage 镜像；应用 SHA-256：`120938417735568354c96e102c3f37456b944d355e0e33b0fc93e29442e6da52`。 | 可以。按同目录 `README.md` 中的 `flash_args` 使用整套匹配文件烧录。 |

## 设备状态与备份

当前设备运行第 2 项 XiaoZhi 独立语音助手。烧录过程重写了 bootloader、分区表、OTA 初始数据、应用和资源分区，没有写 NVS 配置区，因此 Wi-Fi 等已有设置会保留。

刷回官方镜像前的整片备份保存在 `/Users/nnnnzs/.espressif/backups/esp32s3-flash-pre-official-20261024.bin`，SHA-256：`d930471523a8bcdc123894dbeef811cb0b14c6b42c1ebc5bd0edd9fc876b3cbd`。这份 16MiB 快照保存的是更早的 Brookesia 运行状态，不是发布固件，且位于项目目录之外。

刷入官方镜像后的整片回读曾暂存于 `/tmp/esp32s3-flash-current-readback.bin`，SHA-256：`9ec752b7a6f979980b43eeeb8406427a894ab3d408a05915a3efa8181b7c41f4`。它与官方镜像比较时，差异仅在首次启动后更新的 NVS 分区 `0x9000`–`0x997B`；`/tmp` 内容可能被系统清理。设备快照不计入三条固件线。

## 源码与烧录镜像的区别

- `firmware/releases/` 统一保存三条固件线的烧录包；每个版本目录包含对应镜像、烧录参数、说明和校验和。
- Waveshare 官方恢复镜像是官方预编译产物；`vendor/xiaozhi-esp32/` 是独立 XiaoZhi 助手的可编辑源码，不代表官方镜像由当前源码构建。
- `firmware/brookesia-claude/` 是本项目自定义 Brookesia 固件的源码。
- `vendor/waveshare-demo/` 保存开发板示例和参考代码，不是设备当前固件的另一份副本。

ESP-IDF 工具链、Python 环境和 managed component 依赖用于编译源码。使用已归档烧录包时只需要兼容的 esptool，不需要重新下载这些编译依赖。小智烧录包的具体重刷命令见对应目录的 `README.md`。

恢复和备份命令见 [`../recovery/README.md`](../recovery/README.md)，项目固件构建说明见 [`brookesia-claude-build.md`](brookesia-claude-build.md)。
