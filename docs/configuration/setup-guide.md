# Setup Guide

## Overview

Complete setup guide for the T-Watch S3 MAINFRAME firmware — a battery-optimised deep-sleep clock for the LilyGo T-Watch S3 (ESP32-S3).

For a quick-start walkthrough see the [Quick Start Guide](../guides/Quick_Start_Guide.md).  
For the authoritative design reference see the [MAINFRAME Spec](../superpowers/specs/2026-06-04-twatch-mainframe-clock-design.md).

## Prerequisites

### Hardware Requirements
- **LilyGo T-Watch S3** (ESP32-S3, AXP2101, PCF8563, BMA423, FT6X36, ST7789 240×240)
- **USB-C cable** for programming
- **Computer**: Windows, macOS, or Linux with USB support

### Software Requirements
- **Python 3.7+**: Required for PlatformIO
- **PlatformIO**: Recommended build system
- **Git**: For cloning the repository

## Installation

### 1. PlatformIO Setup (Recommended)

#### Install PlatformIO
```bash
pip install platformio

# Verify
python -m platformio --version
```

> Always use `python -m platformio` rather than the `pio` alias — the alias may not be on PATH in all environments.

#### Install Visual Studio Code (Optional)
1. Download [Visual Studio Code](https://code.visualstudio.com/)
2. Install the **PlatformIO IDE** extension (Ctrl+Shift+X → search "PlatformIO IDE")
3. Restart VS Code
4. Open the project folder; PlatformIO will resolve dependencies automatically

#### Clone Repository
```bash
git clone <repository-url>
cd twatch
```

### 2. Arduino IDE Setup (Alternative)

#### Install Arduino IDE
1. Download [Arduino IDE](https://www.arduino.cc/en/software)
2. Install Arduino IDE

#### Install ESP32 Board Package
1. Open Arduino IDE
2. Go to Tools → Board → Boards Manager
3. Search for "ESP32" and install **"ESP32 by Espressif Systems"** version 2.0.9
   > **Important**: Use version 2.0.9; newer versions may have compatibility issues.

#### Configure Board Settings
- **Board**: ESP32S3 Dev Module
- **USB CDC On Boot**: Enable
- **CPU Frequency**: 240MHz
- **Flash Mode**: QIO 80MHz
- **Flash Size**: 16MB (128Mb)
- **PSRAM**: OPI PSRAM
- **Partition Scheme**: 16M Flash (3MB APP / 9.9MB FATFS)
- **USB Mode**: Hardware CDC and JTAG
- **Upload Speed**: 921600

## User Configuration

All user-configurable settings live in `src/config.h`. Edit this file before building:

```cpp
// WiFi credentials — used only for NTP time sync
#define WIFI_SSID     "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"

// Timezone offset from UTC in seconds
// Examples: UTC+0 = 0, UTC-5 = -18000, UTC+1 = 3600
#define TIMEZONE_OFFSET_SEC 0

// Display brightness 0–255
#define DISPLAY_BRIGHTNESS 150

// Screen-on time in milliseconds before deep sleep (default 10 s)
#define SCREEN_TIMEOUT_MS 10000
```

## Build and Flash

### 1. Build Firmware
```bash
python -m platformio run -e twatch-s3
```

### 2. Flash Firmware
```bash
# Replace COM5 with your actual port (Windows) or /dev/ttyACM0 (Linux/macOS)
python -m platformio run -e twatch-s3 -t upload --upload-port COM5
```

### 3. Monitor Serial Output
```bash
python -m platformio device monitor -b 115200 --port COM5
```

### 4. Run Host Unit Tests
```bash
python -m platformio test -e native
```

## Firmware Behaviour

| Action | Result |
|--------|--------|
| Press physical button | Wake from deep sleep; show clock for 10 s |
| Swipe up / down | Cycle pages: CLOCK → SYSTEM → DATE |
| Double-tap screen | Force NTP time sync |
| 10 s of inactivity | Return to deep sleep |
| Cold boot | Connect to WiFi, sync NTP, then deep-sleep |
| Once per day | Background NTP re-sync |

## Project Structure

```
twatch/
├── src/                           # MAINFRAME firmware (single build target)
│   ├── config.h                   # User configuration (WiFi, timezone, etc.)
│   └── *.cpp / *.h                # Firmware source files
├── docs/                          # Documentation
│   ├── superpowers/specs/         # Authoritative design spec
│   ├── superpowers/plans/         # Implementation plan
│   └── ...
├── tools/                         # Development utilities
├── platformio.ini                 # Build system (env: twatch-s3, native)
└── README.md                      # Main project README
```

## Troubleshooting

### Device Not Detected
```bash
# List connected serial devices
python -m platformio device list
```
- Ensure T-Watch is powered on
- Press the crown/button for 1 second to wake
- Check USB cable connection
- Try a different USB port

### Upload Fails
- Verify `upload_port` matches the output of `python -m platformio device list`
- Try pressing the reset button on the watch and re-running the upload command

### Compilation Errors
- Check that `src/config.h` exists and has valid values
- Run `python -m platformio run -e twatch-s3 --verbose` for detailed output

## Support

- [Quick Start Guide](../guides/Quick_Start_Guide.md) — concise first-run walkthrough
- [Troubleshooting Guide](../troubleshooting/troubleshooting.md) — comprehensive problem-solving
- [MAINFRAME Spec](../superpowers/specs/2026-06-04-twatch-mainframe-clock-design.md) — authoritative design reference
- [LilyGO T-Watch S3 Hardware](https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library) — upstream hardware documentation
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/) — ESP32 reference
- [PlatformIO Documentation](https://docs.platformio.org/) — build system reference
