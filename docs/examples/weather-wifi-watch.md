# Weather & WiFi Watch

## Overview
Complete smartwatch firmware with real-time weather data, WiFi power management, and advanced battery optimization.

## Features

### 🌤️ Weather System
- **Real-time Weather**: OpenWeatherMap API integration
- **Automatic Updates**: Configurable update intervals (default: 5 minutes)
- **Temperature Display**: Current temperature with unit selection
- **Weather Description**: Current weather conditions
- **Error Handling**: Robust API error handling and retry logic

### 📶 WiFi Power Management
- **Smart Connection**: Automatic WiFi connection with fallback networks
- **Power Optimization**: Idle disconnect and reconnection logic
- **Battery Saving**: WiFi power management for extended battery life
- **Multiple Networks**: Support for multiple WiFi networks
- **Connection Monitoring**: Real-time connection status

### 🔋 Battery Management
- **Real-time Monitoring**: Battery voltage and percentage
- **Charging Detection**: USB charging status
- **Power Optimization**: CPU frequency and brightness adjustment
- **Sleep Mode**: Automatic sleep after inactivity
- **Power Domains**: Individual power domain management

### 🎨 Display & Interface
- **LovyanGFX**: Modern graphics library integration
- **TrueType Fonts**: High-quality text rendering
- **Touch Interface**: Responsive touch controls
- **Color Coding**: Status-based color indicators
- **PNG Icons**: High-quality WiFi and status icons

## Quick Start

### 1. Upload Firmware
```bash
cd examples/demo/LovyanGFX_Watch/
pio run -e twatch-s3 --target upload
```

### 2. Configure Weather
Create `data/weather_config.ini`:
```ini
# Weather API Configuration
api_key=YOUR_OPENWEATHERMAP_API_KEY
city=Your City Name
country=BR
update_interval=300
units=metric
```

### 3. Configure WiFi
Create `data/wifi_config.ini`:
```ini
# WiFi Configuration
ssid1=YOUR_WIFI_SSID
password1=YOUR_WIFI_PASSWORD
ssid2=YOUR_BACKUP_WIFI_SSID
password2=YOUR_BACKUP_WIFI_PASSWORD
```

### 4. Upload Filesystem
```bash
pio run -e twatch-s3 --target uploadfs
```

## Configuration

### Weather Settings
| Parameter | Description | Default |
|-----------|-------------|---------|
| `api_key` | OpenWeatherMap API key | Required |
| `city` | City name (no quotes) | Required |
| `country` | Country code (BR, US, etc.) | Required |
| `update_interval` | Update interval in seconds | 300 |
| `units` | Temperature units (metric/imperial) | metric |

### WiFi Settings
| Parameter | Description | Default |
|-----------|-------------|---------|
| `ssid1` | Primary WiFi network | Required |
| `password1` | Primary WiFi password | Required |
| `ssid2` | Backup WiFi network | Optional |
| `password2` | Backup WiFi password | Optional |

### Power Management
| Parameter | Description | Default |
|-----------|-------------|---------|
| `SLEEP_TIMEOUT` | Sleep timeout in milliseconds | 15000 |
| `WIFI_IDLE_TIMEOUT` | WiFi idle timeout | 30000 |
| `WIFI_RECONNECT_INTERVAL` | Reconnection interval | 60000 |

## Usage

### Display Information
- **Time**: 24-hour format with seconds
- **Date**: Day/Month format
- **Day**: Day of week abbreviation
- **Battery**: Percentage with color coding
- **WiFi**: Connection status with icon
- **Steps**: Step counter (if available)
- **Weather**: Current temperature

### Touch Controls
- **Tap**: Wake from sleep mode
- **Long Press**: Force sleep mode
- **Auto-sleep**: 15 seconds of inactivity

### Power States
- **Active**: Full brightness and CPU frequency
- **Sleep**: Reduced brightness and CPU frequency
- **Deep Sleep**: Minimal power consumption

## Troubleshooting

### Weather Issues
1. **Weather shows 0.0°C**
   - Check API key in `weather_config.ini`
   - Verify city name has no quotes
   - Ensure WiFi is connected

2. **Weather not updating**
   - Check `update_interval` setting
   - Monitor serial output for API errors
   - Verify OpenWeatherMap API key

### WiFi Issues
1. **WiFi not connecting**
   - Check SSID and password in `wifi_config.ini`
   - Verify networks are in range
   - Check serial output for connection errors

2. **WiFi disconnects frequently**
   - Adjust `WIFI_IDLE_TIMEOUT` setting
   - Check signal strength
   - Verify power management settings

### Battery Issues
1. **Battery drains quickly**
   - Check sleep timeout settings
   - Verify WiFi power management
   - Monitor CPU frequency settings

2. **Charging not detected**
   - Check USB connection
   - Verify PMU initialization
   - Check power domain settings

## API Reference

### Weather Functions
```cpp
void getWeatherData()           // Fetch weather from API
bool loadWeatherConfig()        // Load weather configuration
void processWeatherResponse()   // Process API response
```

### WiFi Functions
```cpp
bool connectWiFi()              // Connect to WiFi
void manageWiFiPower()          // Manage WiFi power
void disconnectWiFi()           // Disconnect WiFi
```

### Battery Functions
```cpp
void readBatteryInfo()          // Read battery status
void optimizeBatteryUsage()     // Optimize power consumption
void manageSleepMode()          // Manage sleep states
```

## Serial Debug Output
The firmware provides detailed debug information via serial:
```
Weather check: millis()=12800, lastWeatherUpdate=0, interval=300000
Getting weather data...
City: London, Country: GB, Units: metric
Weather API URL: http://api.openweathermap.org/data/2.5/weather?q=London,GB&appid=...
HTTP response code: 200
✅ Weather temperature: 18.5°C
```

## Performance
- **RAM Usage**: ~17% (56KB of 320KB)
- **Flash Usage**: ~33% (1MB of 3MB)
- **Battery Life**: 24-48 hours depending on usage
- **Update Interval**: 5 minutes (configurable)
- **Sleep Timeout**: 15 seconds (configurable) 