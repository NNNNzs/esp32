# 固件烧录包目录

本目录统一保存 Waveshare ESP32-S3-Touch-LCD-1.83 的三条固件线。`vendor/` 保存可参考或用于构建的源码副本；这里保存可直接烧录的镜像包。

| 固件 | 源码或来源 | 烧录包目录 | 用途 |
| --- | --- | --- | --- |
| Waveshare 官方恢复固件 | Waveshare 提供的预编译镜像，未公开对应的确切源码版本 | `waveshare-official-xiaozhi-20251024/` | 恢复官方提供的整片 Flash 镜像 |
| XiaoZhi 2.2.6 | `vendor/xiaozhi-esp32/` | `xiaozhi-2.2.6-waveshare-esp32-s3-touch-lcd-1.83-20261024/` | 独立小智语音助手 |
| Brookesia Claude | `firmware/brookesia-claude/` | `brookesia-claude-20260528/` | 多应用启动器及 Claude Code 状态显示 |

每个版本目录都含有 `README.md`，说明对应固件的板型、镜像和烧录方法；`SHA256SUMS` 用于核对固件镜像和分区输入文件。源码构建包中的 `flash_args` 记录匹配的烧录地址和参数。

ESP-IDF 工具链、Python 环境和组件依赖用于从源码编译。烧录已归档的固件包只需要 esptool 和该目录中的文件，不需要重新下载或编译这些依赖。
