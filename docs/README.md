# Edward Watch Documentation

## Overview
Complete documentation for Edward Watch, a smartwatch firmware based on LilyGO T-Watch S3.

📋 **[Complete Documentation Index](DOCUMENTATION_INDEX.md)** - Find all documentation files

## Quick Navigation
- **[Documentation Index](DOCUMENTATION_INDEX.md)** - Complete list of all documentation
- **[Setup Guide](configuration/setup-guide.md)** - Get started quickly
- **[Troubleshooting](troubleshooting/troubleshooting.md)** - Solve common issues

## Quick Start
- [Setup Guide](configuration/setup-guide.md) - Complete installation and configuration guide
- [Weather & WiFi Watch](examples/weather-wifi-watch.md) - Main smartwatch firmware
- [BLE Notifications](examples/ble-notifications.md) - Smartphone notification receiver

## Documentation Structure

### 📚 Configuration
- [Setup Guide](configuration/setup-guide.md) - Installation, configuration, and development setup

### 📱 Examples
- [Weather & WiFi Watch](examples/weather-wifi-watch.md) - Complete smartwatch with weather, WiFi, and battery management
- [BLE Notifications](examples/ble-notifications.md) - Receive smartphone notifications via BLE
- [Android App Guide](examples/android-app-guide.md) - Create Android app for notification forwarding

### 🛠️ Troubleshooting
- [Troubleshooting Guide](troubleshooting/troubleshooting.md) - Comprehensive problem-solving guide

### 🏗️ Hardware Documentation
- [Shell Design](shell/) - 3D shell files and case design
- [Schematics](schematic/) - Hardware schematics and diagrams
- [Images](images/) - Documentation images and screenshots

### 🔧 Development Tools
- [MCP Organisation](MCP_ORGANISATION.md) - MCP configuration and setup
- [Chinese Documentation](README_CN.MD) - Chinese language documentation

## Features

### 🌤️ Weather System
- Real-time weather data from OpenWeatherMap API
- Configurable update intervals
- Temperature display with unit selection
- Error handling and retry logic

### 📶 WiFi Power Management
- Smart WiFi connection with fallback networks
- Power optimization for extended battery life
- Automatic idle disconnect and reconnection
- Multiple network support

### 🔋 Battery Management
- Real-time battery monitoring
- USB charging detection
- Power optimization and sleep modes
- Individual power domain management

### 📱 BLE Notifications
- Receive smartphone notifications without additional apps
- Android Notification Access Service integration
- Touch navigation through notifications
- Notification history management

### 🎨 Display & Interface
- LovyanGFX graphics library
- TrueType font support
- Responsive touch interface
- Color-coded status indicators

## Getting Started

### 1. Choose Your Example
- **Weather & WiFi Watch**: Complete smartwatch with weather and WiFi
- **BLE Notifications**: Smartphone notification receiver
- **Custom Development**: Start with existing examples

### 2. Setup Development Environment
1. Install [PlatformIO](https://platformio.org/) (recommended)
2. Clone the repository
3. Open project in VS Code with PlatformIO extension
4. Install dependencies

### 3. Configure Your Watch
1. Create configuration files in `data/` directory
2. Upload firmware: `pio run -e twatch-s3 --target upload`
3. Upload filesystem: `pio run -e twatch-s3 --target uploadfs`
4. Monitor serial output: `pio device monitor`

## Configuration Files

### Weather Configuration (`data/weather_config.ini`)
```ini
api_key=YOUR_OPENWEATHERMAP_API_KEY
city=Your City Name
country=BR
update_interval=300
units=metric
```

### WiFi Configuration (`data/wifi_config.ini`)
```ini
ssid1=YOUR_WIFI_SSID
password1=YOUR_WIFI_PASSWORD
ssid2=YOUR_BACKUP_WIFI_SSID
password2=YOUR_BACKUP_WIFI_PASSWORD
```

## Development

### Project Structure
```
twatch/
├── docs/                    # This documentation
│   ├── README.md           # Documentation index
│   ├── README_CN.MD        # Chinese documentation
│   ├── MCP_ORGANISATION.md # MCP configuration
│   ├── configuration/      # Setup guides
│   ├── examples/           # Example documentation
│   ├── troubleshooting/    # Problem-solving guides
│   ├── shell/              # 3D shell files
│   ├── schematic/          # Hardware schematics
│   └── images/             # Documentation images
├── examples/               # Example projects
│   ├── demo/
│   │   ├── LovyanGFX_Watch/    # Weather & WiFi Watch
│   │   └── NotificationBLE/    # BLE Notifications
│   ├── display/            # Display examples
│   ├── peripheral/         # Peripheral examples
│   └── sensor/            # Sensor examples
├── src/                   # Source code
├── data/                  # Configuration files
├── assets/                # Project assets
├── board/                 # Board configuration
├── extras/                # Additional headers
├── firmware/              # Firmware files
├── mcp/                   # MCP configuration
├── platformio.ini         # PlatformIO configuration
└── README.md             # Main project README
```

### Adding New Features
1. Create new example in `examples/` directory
2. Update documentation in `docs/`
3. Test thoroughly before committing
4. Update main README.md with new features

### Best Practices
- Keep examples separate and focused
- Use configuration files for settings
- Implement proper error handling
- Document all features and configurations
- Test thoroughly before deployment

## Support

### Documentation
- [Setup Guide](configuration/setup-guide.md) - Complete setup instructions
- [Troubleshooting](troubleshooting/troubleshooting.md) - Problem-solving guide
- [Examples](examples/) - Detailed example documentation

### Resources
- [LilyGO T-Watch S3](https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library) - Original hardware documentation
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/) - ESP32 reference
- [PlatformIO Documentation](https://docs.platformio.org/) - Development environment

### Community
- [GitHub Issues](https://github.com/your-repo/edward-watch/issues) - Report bugs and request features
- [Discussions](https://github.com/your-repo/edward-watch/discussions) - Community discussions
- [Documentation](https://github.com/your-repo/edward-watch/docs) - This documentation

## Contributing

### Guidelines
1. Follow existing code style and conventions
2. Add documentation for new features
3. Test thoroughly before submitting
4. Update relevant documentation
5. Include examples for new features

### Development Workflow
1. Fork the repository
2. Create feature branch
3. Make changes and test
4. Update documentation
5. Submit pull request

## License
This project is open source and available under the MIT License.

## Acknowledgments
- Based on [LilyGO T-Watch S3](https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library)
- Uses [LovyanGFX](https://github.com/lovyan03/LovyanGFX) for graphics
- Weather data from [OpenWeatherMap](https://openweathermap.org/)
- BLE implementation using [NimBLE](https://github.com/h2zero/NimBLE-Arduino) 