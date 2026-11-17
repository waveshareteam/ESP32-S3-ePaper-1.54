[English](README.md)

# 出厂固件

请按硬件版本选择 BIN。背面标有 V2 或 PCB 左上角有 V2 丝印的板子为 V2，其余为 V1。两个版本的固件不可互刷。

| 出厂固件 | 硬件版本 | 烧录偏移 |
| --- | --- | --- |
| [ESP32-S3-ePaper-1.54_FactoryOnly_20260416.bin](ESP32-S3-ePaper-1.54_FactoryOnly_20260416.bin) | V2 | `0x0` |
| [ESP32-S3-ePaper-1.54_FactoryOnly_20260415.bin](ESP32-S3-ePaper-1.54_FactoryOnly_20260415.bin) | V1 | `0x0` |

使开发板进入下载模式，将对应的合并 BIN 从 `0x0` 烧录。
