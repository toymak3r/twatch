# T-Watch S3 Quick Start Guide

Get the MAINFRAME firmware running on your T-Watch S3 in minutes.

## What You'll Learn

- Set up the development environment
- Build and flash the single MAINFRAME firmware
- Configure WiFi credentials and timezone
- Understand the clock's behaviour and gestures

## Prerequisites

### Hardware Required
- **LilyGo T-Watch S3** (ESP32-S3, AXP2101, PCF8563, BMA423, FT6X36, ST7789 240×240)
- **USB-C cable** for programming
- **Computer** with internet access

### Software Required
- **Python 3.7+** (required for PlatformIO)
- **PlatformIO** (installed via pip)
- **Git** (for cloning the repository)

## Step 1: Install PlatformIO

```bash
pip install platformio

# Verify installation
python -m platformio --version
```

> Note: Use `python -m platformio` rather than the `pio` alias — the alias may not be on PATH in all environments.

## Step 2: Get the Code

```bash
git clone <repository-url>
cd twatch
```

## Step 3: Configure the Firmware

Edit `src/config.h` to set your credentials and preferences:

```cpp
// WiFi — used only for NTP time sync
#define WIFI_SSID     "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"

// Timezone offset from UTC in seconds (e.g. -18000 = UTC-5)
#define TIMEZONE_OFFSET_SEC -18000

// Screen brightness 0–255
#define DISPLAY_BRIGHTNESS 150

// Screen-on time in milliseconds before deep sleep (default: 10 s)
#define SCREEN_TIMEOUT_MS 10000
```

## Step 4: Connect the Watch

1. Connect the USB-C cable to the watch
2. Identify the serial port:
   ```bash
   # Windows PowerShell
   python -m platformio device list
   # Look for a COM port, e.g. COM5
   ```

## Step 5: Build and Flash

```bash
# Build
python -m platformio run -e twatch-s3

# Flash (replace COM5 with your port)
python -m platformio run -e twatch-s3 -t upload --upload-port COM5
```

You should see `SUCCESS` when the upload completes.

## Step 6: Verify

Press the physical button on the watch. The CRT-style clock face should appear for 10 seconds, then the watch returns to deep sleep.

### Monitor Serial Output (Optional)
```bash
python -m platformio device monitor -b 115200 --port COM5
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

## Run Host Unit Tests

```bash
python -m platformio test -e native
```

## Common Issues

### Upload Fails — Port Not Found
- Disconnect and reconnect the USB cable
- Press the reset button on the watch
- Try a different USB port
- Re-run `python -m platformio device list` to confirm the port name

### Display Stays Blank
- Verify `DISPLAY_BRIGHTNESS` in `src/config.h` is not `0`
- Press and hold the physical button for 2 seconds to force a wake

### Time Is Wrong
- Check `TIMEZONE_OFFSET_SEC` in `src/config.h`
- Double-tap the screen to trigger an immediate NTP sync
- Verify WiFi credentials are correct

### Battery Drains Faster Than Expected
- Reduce `SCREEN_TIMEOUT_MS` in `src/config.h` (e.g., 5000 for 5 s)
- The firmware spends most of its life in deep sleep; battery drain is dominated by wake frequency

## Next Steps

- **[Setup Guide](../configuration/setup-guide.md)** — full development environment walkthrough
- **[MAINFRAME Spec](../superpowers/specs/2026-06-04-twatch-mainframe-clock-design.md)** — authoritative design reference
- **[Battery Optimization Guide](../BATTERY_OPTIMIZATION.md)** — general battery-saving strategies
- **[Hardware Reference](../hardware/TWatch_S3_Hardware.md)** — pinouts and component specs
- **[Troubleshooting Guide](../troubleshooting/troubleshooting.md)** — comprehensive problem-solving

---

**Your T-Watch S3 is now running the MAINFRAME firmware!**
