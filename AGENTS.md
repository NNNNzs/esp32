# AGENTS.md

Project entry points and working notes for this repository.

## Start Here

- [README.md](./README.md): main project overview, build instructions, device setup, hook mapping, recovery steps, and source references.

## Repository Map

- `firmware/`: current ESP-IDF firmware and related app code.
- `claudecode_status_display/`: early Arduino fallback sketch.
- `tools/`: PC-side scripts for building, flashing, and status publishing.
- `recovery/`: restore and flash backup scripts plus official firmware image.
- `docs/`: project notes and offline material index.
- `vendor/`: Waveshare and XiaoZhi reference sources.

## Notes

- Current target board: Waveshare `ESP32-S3-Touch-LCD-1.83`.
- Mainline firmware lives in `firmware/brookesia-claude`.
- If you are looking for setup or troubleshooting details, check `README.md` first.
