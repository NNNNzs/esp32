# AGENTS.md

本文件列出项目入口、目录索引和工作说明。

## 从这里开始

- [README.md](./README.md)：项目概览、固件分类、构建说明、设备配置、恢复步骤和源码位置。

## 目录索引

- `firmware/brookesia-claude/`：本项目 Brookesia Claude 固件源码。
- `firmware/releases/`：官方、小智和 Brookesia 三条固件线的烧录包，附说明和校验和。
- `claudecode_status_display/`：早期 Arduino 状态显示草稿。
- `tools/`：电脑端构建、烧录和状态发布脚本。
- `recovery/`：恢复与整片 Flash 备份脚本。
- `docs/`：项目文档和离线资料索引。
- `vendor/`：外部项目的源码副本和参考示例，不是烧录包目录。

## 项目信息

- 目标开发板：Waveshare `ESP32-S3-Touch-LCD-1.83`。
- 项目主线固件源码：`firmware/brookesia-claude/`。
- 设置和故障排查入口：先看 `README.md`。
