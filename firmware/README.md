[中文](README_ZH.md)

# Factory firmware

Use the BIN for your board revision. V2 boards have a V2 mark on the back or at the upper left of the PCB. Boards without the mark are V1. The firmware is specific to each revision.

| Factory firmware | Hardware revision | Flash offset |
| --- | --- | --- |
| [ESP32-S3-ePaper-1.54_FactoryOnly_20260416.bin](ESP32-S3-ePaper-1.54_FactoryOnly_20260416.bin) | V2 | `0x0` |
| [ESP32-S3-ePaper-1.54_FactoryOnly_20260415.bin](ESP32-S3-ePaper-1.54_FactoryOnly_20260415.bin) | V1 | `0x0` |

Enter download mode and write the selected merged BIN at `0x0`.

Both images were originally dated 2026-04-16. The V1 filename uses 2026-04-15 as a catalog version date to distinguish it from V2. The V1 binary is unchanged.
