# Configuration Files Setup

This directory contains configuration templates for the T-Watch S3.

## Setup Instructions

### 1. WiFi Configuration
Copy `wifi_config_template.ini` to `wifi_config.ini` and fill in your WiFi credentials:

```bash
cp data/wifi_config_template.ini data/wifi_config.ini
```

Edit `data/wifi_config.ini`:
```ini
wifi1_ssid=YourWiFiNetwork
wifi1_password=YourWiFiPassword
```

### 2. Weather Configuration
Copy `weather_config_template.ini` to `weather_config.ini` and fill in your OpenWeatherMap API key:

```bash
cp data/weather_config_template.ini data/weather_config.ini
```

Edit `data/weather_config.ini`:
```ini
api_key=your_openweathermap_api_key_here
city=Sao Paulo
country=BR
units=metric
update_interval=1800
```

**Get API Key:** Sign up at https://openweathermap.org/api (free tier available)

### 3. Upload Filesystem
After creating your config files, upload them to the watch:

```bash
pio run -t uploadfs --upload-port /dev/ttyACM1
```

Or if using /dev/ttyACM0:
```bash
pio run -t uploadfs --upload-port /dev/ttyACM0
```

## Files Structure on Watch

After upload, the files will be in the root of the FATFS filesystem:
- `/wifi_config.ini` - WiFi network credentials
- `/weather_config.ini` - Weather API configuration

The watch firmware will automatically load these files on boot.
