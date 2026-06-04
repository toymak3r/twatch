# T-Watch S3 Documentation

## Overview

Documentation for the **MAINFRAME firmware** — a single, battery-optimised deep-sleep clock for the LilyGo T-Watch S3 (ESP32-S3, AXP2101, PCF8563, BMA423, FT6X36, ST7789 240×240).

See the [Documentation Index](index.md) for a full map of all docs.

## Quick Navigation

- **[Quick Start Guide](guides/Quick_Start_Guide.md)** — build, flash, and configure the firmware
- **[Setup Guide](configuration/setup-guide.md)** — development environment and build commands
- **[Troubleshooting](troubleshooting/troubleshooting.md)** — solve common issues
- **[Hardware Reference](hardware/TWatch_S3_Hardware.md)** — pinouts, components, schematics

## Design References

- **[MAINFRAME Firmware Spec](superpowers/specs/2026-06-04-twatch-mainframe-clock-design.md)** — authoritative hardware & software design
- **[MAINFRAME Firmware Plan](superpowers/plans/2026-06-04-twatch-mainframe-firmware.md)** — implementation plan and task breakdown

## Firmware Summary

The project builds a **single firmware** (`src/`):

| Property | Value |
|----------|-------|
| Target hardware | LilyGo T-Watch S3 (ESP32-S3) |
| PlatformIO env | `twatch-s3` |
| Wake source | Physical button |
| Screen-on time | 10 seconds per wake |
| WiFi usage | NTP only (cold boot + once/day) |
| User config | `src/config.h` |

### Build & Flash
```bash
# Build
python -m platformio run -e twatch-s3

# Flash (replace COM5 with your port)
python -m platformio run -e twatch-s3 -t upload --upload-port COM5

# Host unit tests
python -m platformio test -e native
```

## Documentation Structure

```
docs/
├── README.md                      # This file
├── index.md                       # Full documentation index
├── DOCUMENTATION_INDEX.md         # Alphabetical file index
├── BATTERY_OPTIMIZATION.md        # General battery-saving guide
├── api/LilyGoLib_API.md           # Legacy API reference (kept for reference)
├── configuration/setup-guide.md   # Dev environment setup
├── guides/Quick_Start_Guide.md    # Quick start
├── hardware/TWatch_S3_Hardware.md # Hardware specs (still valid)
├── superpowers/specs/             # Authoritative design spec
├── superpowers/plans/             # Implementation plan
├── troubleshooting/               # Troubleshooting guide
├── schematic/                     # Hardware schematics
├── shell/                         # 3D shell files
└── images/                        # Documentation images
```

## Support

- [GitHub Issues](https://github.com/your-repo/twatch/issues) — bug reports and feature requests
- [Discussions](https://github.com/your-repo/twatch/discussions) — community discussions
