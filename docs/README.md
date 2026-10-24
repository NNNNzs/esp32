# 项目文档索引

本目录存放项目维护的说明文档和离线资料索引。固件镜像与源码分别归档在 `firmware/releases/` 和各自源码目录。

## 项目说明

- [`../README.md`](../README.md)：项目入口、三条固件线、目录结构、构建、设备设置和恢复说明。
- [`firmware-inventory.md`](firmware-inventory.md)：固件来源、烧录包、设备状态和备份清单。
- [`../firmware/releases/README.md`](../firmware/releases/README.md)：官方、小智和 Brookesia 烧录包目录。
- [`brookesia-claude-build.md`](brookesia-claude-build.md)：Brookesia 固件的构建环境、兼容修复和烧录记录。
- [`xiaozhi-source.md`](xiaozhi-source.md)：小智源码、官方镜像和板级配置说明。
- [`multi-app-claude-mqtt-plan.md`](multi-app-claude-mqtt-plan.md)：Brookesia 多应用与 MQTT Claude 状态功能的架构方案。
- [`../firmware/brookesia-claude/README.md`](../firmware/brookesia-claude/README.md)：Brookesia 固件功能、配置和构建说明。
- [`../recovery/README.md`](../recovery/README.md)：官方镜像恢复和设备 Flash 备份说明。
- [`../tools/build_firmware.py`](../tools/build_firmware.py)：跨平台固件构建和烧录脚本。
- [`../tools/rebuild_generated.py`](../tools/rebuild_generated.py)：重新生成离线资料和固件资源的脚本。
- [`../tools/fetch_reference_sources.py`](../tools/fetch_reference_sources.py)：按需获取参考源码。

## 离线资料

Waveshare Wiki 镜像位于：

```text
docs/waveshare-esp32-s3-touch-lcd-1.83/
```

该目录由 `tools/offline_waveshare_wiki.py` 生成，并在 `.gitignore` 中排除。需要离线副本时再运行生成脚本。
