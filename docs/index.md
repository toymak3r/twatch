# T-Watch S3 Documentation Index

Complete documentation for the MAINFRAME firmware — a battery-optimised deep-sleep clock for the LilyGo T-Watch S3.

## Quick Start

### [Quick Start Guide](guides/Quick_Start_Guide.md)
Get the MAINFRAME firmware running in minutes.

**What you'll learn:**
- Set up the development environment
- Build and flash the firmware
- Configure WiFi credentials and timezone
- Understand the clock's behaviour and gestures

## Hardware Documentation

### [Hardware Reference](hardware/TWatch_S3_Hardware.md)
Complete T-Watch S3 hardware specifications, pinouts, and electrical characteristics.

**Contents:**
- Component specifications (ESP32-S3, AXP2101, PCF8563, BMA423, FT6X36, ST7789)
- Complete pinout diagram with GPIO mappings
- Power management rail configuration
- Electrical requirements and troubleshooting

## API Reference

### [LilyGoLib API Documentation](api/LilyGoLib_API.md)
Historical reference for the removed LilyGoLib wrapper (kept for reference; not the current API).

## Battery Optimization

### [Battery Optimization Guide](../BATTERY_OPTIMIZATION.md)
General battery-saving strategies applicable to the T-Watch S3 platform.

**Key techniques used by the MAINFRAME firmware:**
- ESP32-S3 deep sleep between interactions
- Physical button wake-up (no continuous polling)
- 10-second screen timeout before returning to deep sleep
- WiFi used only for NTP (cold boot + once per day)

## Development Tools

### [Development Tools Guide](../tools/Development_Tools.md)
Development utilities and scripts for T-Watch S3 development.

## Design References

- **[MAINFRAME Firmware Spec](superpowers/specs/2026-06-04-twatch-mainframe-clock-design.md)** — authoritative hardware & software design
- **[MAINFRAME Firmware Plan](superpowers/plans/2026-06-04-twatch-mainframe-firmware.md)** — implementation plan and task breakdown

## The MAINFRAME Firmware

The project contains a **single firmware** targeting the LilyGo T-Watch S3 (ESP32-S3).

### Behaviour
- Wakes from deep sleep when the physical button is pressed
- Displays a CRT-style clock for 10 seconds, then returns to deep sleep
- Touch gestures: swipe up/down cycles pages (CLOCK → SYSTEM → DATE), double-tap forces an NTP sync
- WiFi connects only for NTP: once on cold boot and once per day thereafter

### Build & Flash
```bash
# Build
python -m platformio run -e twatch-s3

# Flash (replace COM5 with your port)
python -m platformio run -e twatch-s3 -t upload --upload-port COM5
```

### Run Host Unit Tests
```bash
python -m platformio test -e native
```

### User Configuration
Edit `src/config.h` to set WiFi credentials, timezone, display brightness, and screen timeout.

## Troubleshooting

### Upload Problems
```bash
# List available serial ports (Windows PowerShell)
python -m platformio device list
```

### Display Issues
- Verify brightness setting in `src/config.h`
- Press and hold the physical button to force a wake

### Battery Issues
- The firmware spends nearly all its time in deep sleep — battery life is dominated by how often you wake it
- Reduce the screen-on timeout in `src/config.h` if battery drain is too high

### Sensor Issues
- BMA423 accelerometer at I2C address 0x19
- PCF8563 RTC at I2C address 0x51
- FT6X36 touch controller at I2C address 0x38

## File Structure

```
twatch/
├── src/                           # MAINFRAME firmware (single build target)
│   ├── config.h                   # User configuration (WiFi, timezone, etc.)
│   └── *.cpp / *.h                # Firmware source files
├── docs/                          # Documentation
│   ├── api/LilyGoLib_API.md       # Legacy API reference (removed wrapper)
│   ├── hardware/TWatch_S3_Hardware.md  # Hardware specs
│   ├── guides/Quick_Start_Guide.md     # Quick start
│   ├── superpowers/specs/         # Authoritative design spec
│   ├── superpowers/plans/         # Implementation plan
│   └── index.md                   # This file
├── tools/                         # Development utilities
├── platformio.ini                 # Build system (env: twatch-s3, native)
└── README.md                      # Main project README
```

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-change`
3. Make changes and run host tests: `python -m platformio test -e native`
4. Submit a pull request with a clear description

### Guidelines
- Keep it lean — no new dependencies unless essential
- Document everything — update relevant docs
- Test battery impact — the deep-sleep model must be preserved
- Follow existing code style
