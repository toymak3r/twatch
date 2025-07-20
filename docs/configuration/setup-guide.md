# Setup Guide

## Overview
Complete setup guide for Edward Watch based on LilyGO T-Watch S3.

## Prerequisites

### Hardware Requirements
- **T-Watch S3**: LilyGO T-Watch S3 smartwatch
- **USB Cable**: Micro-USB cable for programming
- **Computer**: Windows, macOS, or Linux with USB support

### Software Requirements
- **PlatformIO**: Recommended development environment
- **Arduino IDE**: Alternative development environment
- **Python**: Required for PlatformIO (3.7+)

## Installation

### 1. PlatformIO Setup (Recommended)

#### Install Visual Studio Code
1. Download [Visual Studio Code](https://code.visualstudio.com/)
2. Install and launch VS Code

#### Install PlatformIO Extension
1. Open VS Code Extensions (Ctrl+Shift+X)
2. Search for "PlatformIO IDE"
3. Install the extension
4. Restart VS Code

#### Clone Repository
```bash
git clone https://github.com/your-repo/edward-watch.git
cd edward-watch
```

#### Open Project
1. In VS Code: File → Open Folder
2. Select the `edward-watch` directory
3. Wait for PlatformIO to install dependencies

### 2. Arduino IDE Setup (Alternative)

#### Install Arduino IDE
1. Download [Arduino IDE](https://www.arduino.cc/en/software)
2. Install Arduino IDE

#### Install ESP32 Board Package
1. Open Arduino IDE
2. Go to Tools → Board → Boards Manager
3. Search for "ESP32"
4. Install "ESP32 by Espressif Systems" version 2.0.9
5. **Important**: Use version 2.0.9, newer versions may have compatibility issues

#### Install Required Libraries
1. Download [T-Watch-Deps](https://github.com/Xinyuan-LilyGO/T-Watch-Deps)
2. Copy all directories to `<Arduino/libraries>` folder
3. Install [TTGO_TWatch_Library](https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library)

#### Configure Board Settings
- **Board**: ESP32S3 Dev Module
- **USB CDC On Boot**: Enable
- **CPU Frequency**: 240MHz
- **USB DFU On Boot**: Disable
- **Flash Mode**: QIO 80MHz
- **Flash Size**: 16MB(128Mb)
- **USB Firmware MSC On Boot**: Disable
- **PSRAM**: OPI PSRAM
- **Partition Scheme**: 16M Flash(3MB APP/9.9MB FATFS)
- **USB Mode**: Hardware CDC and JTAG
- **Upload Mode**: UART0/Hardware CDC
- **Upload Speed**: 921600

## Configuration Files

### Weather Configuration
Create `data/weather_config.ini`:
```ini
# Weather API Configuration
# Get free API key from https://openweathermap.org/api
api_key=YOUR_OPENWEATHERMAP_API_KEY

# City name (no quotes, use URL encoding for spaces)
city=Your City Name

# Country code (BR, US, GB, etc.)
country=BR

# Update interval in seconds (300 = 5 minutes)
update_interval=300

# Temperature units (metric, imperial, kelvin)
units=metric
```

### WiFi Configuration
Create `data/wifi_config.ini`:
```ini
# WiFi Configuration
# Primary WiFi network
ssid1=YOUR_WIFI_SSID
password1=YOUR_WIFI_PASSWORD

# Backup WiFi network (optional)
ssid2=YOUR_BACKUP_WIFI_SSID
password2=YOUR_BACKUP_WIFI_PASSWORD

# Additional networks (optional)
ssid3=YOUR_THIRD_WIFI_SSID
password3=YOUR_THIRD_WIFI_PASSWORD
```

## Upload Process

### 1. Upload Firmware
```bash
# For Weather & WiFi Watch
cd examples/demo/LovyanGFX_Watch/
pio run -e twatch-s3 --target upload

# For BLE Notifications
cd examples/demo/NotificationBLE/
pio run -e twatch-s3 --target upload -f notification_ble.ino
```

### 2. Upload Filesystem
```bash
# Upload configuration files
pio run -e twatch-s3 --target uploadfs
```

### 3. Monitor Serial Output
```bash
# Monitor serial output for debugging
pio device monitor
```

## Troubleshooting

### Upload Issues
1. **Device not detected**
   - Ensure T-Watch is powered on
   - Press crown button for 1 second to wake
   - Check USB cable connection
   - Try different USB port

2. **Upload fails**
   - Check board settings in platformio.ini
   - Verify correct port selection
   - Try manual boot mode (see FAQ)

3. **Compilation errors**
   - Check library dependencies
   - Verify ESP32 board package version
   - Check for conflicting libraries

### Configuration Issues
1. **Weather not working**
   - Verify API key is valid
   - Check city name format (no quotes)
   - Ensure WiFi is connected
   - Monitor serial output for errors

2. **WiFi not connecting**
   - Check SSID and password
   - Verify network is in range
   - Check filesystem was uploaded
   - Monitor serial output

3. **Filesystem upload fails**
   - Check data folder exists
   - Verify configuration files are correct
   - Try manual filesystem upload

## Development

### Project Structure
```
edward-watch/
├── docs/                    # Documentation
├── examples/               # Example projects
│   ├── demo/
│   │   ├── LovyanGFX_Watch/    # Weather & WiFi Watch
│   │   └── NotificationBLE/    # BLE Notifications
│   ├── display/            # Display examples
│   ├── peripheral/         # Peripheral examples
│   └── sensor/            # Sensor examples
├── src/                   # Source code
├── data/                  # Configuration files
├── platformio.ini         # PlatformIO configuration
└── README.md             # Main documentation
```

### Adding New Features
1. Create new example in `examples/` directory
2. Update documentation in `docs/`
3. Test thoroughly before committing
4. Update README.md with new features

### Debugging
1. Use serial monitor for debug output
2. Check configuration files
3. Verify hardware connections
4. Monitor power consumption

## Support

### Documentation
- [Weather & WiFi Watch](weather-wifi-watch.md)
- [BLE Notifications](ble-notifications.md)
- [Troubleshooting Guide](../troubleshooting/troubleshooting.md)

### Resources
- [LilyGO T-Watch S3 Documentation](https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library)
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [PlatformIO Documentation](https://docs.platformio.org/)

### Community
- [GitHub Issues](https://github.com/your-repo/edward-watch/issues)
- [Discussions](https://github.com/your-repo/edward-watch/discussions) 