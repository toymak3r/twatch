# 🚀 T-Watch S3 Quick Start Guide

Get your T-Watch S3 running in 5 minutes with optimized battery life!

## 🎯 What You'll Learn

- Set up development environment
- Flash your first optimized firmware  
- Achieve 24+ hour battery life
- Use essential smartwatch features

## 📋 Prerequisites

### Hardware Required
- **T-Watch S3** smartwatch
- **USB-C cable** for programming
- **Computer** with internet access

### Software Required
- **PlatformIO** (recommended) or Arduino IDE
- **Git** (for cloning repository)

## ⚡ 5-Minute Quick Start

### Step 1: Install PlatformIO
```bash
# Install Python (if not already installed)
python3 --version

# Install PlatformIO
pip install platformio

# Verify installation
pio --version
```

### Step 2: Get the Code
```bash
# Clone the optimized repository
git clone <repository-url>
cd twatch

# Verify structure
ls -la
# Should see: src/, examples/, platformio.ini, README.md
```

### Step 3: Connect T-Watch S3
1. Connect USB-C cable to watch
2. Verify device appears:
   ```bash
   ls /dev/ttyACM*
   # Should see: /dev/ttyACM1 (or similar)
   ```

### Step 4: Flash Battery Optimization
```bash
# Use the 24+ hour battery optimization
cp examples/essential/BatteryOptimization/* src/

# Build and upload
pio run --target upload --upload-port /dev/ttyACM1

# Wait for upload to complete (should show "SUCCESS")
```

### Step 5: Verify Success
```bash
# Monitor serial output
pio device monitor -b 115200

# You should see:
# === T-Watch Battery Optimization ===
# Boot #1 - Applying maximum power saving
# 🔋 Applying maximum battery optimization...
# ✅ Battery optimization active
# 🎯 Target: 24+ hours battery life
```

## 🎉 Congratulations! 

Your T-Watch S3 is now running optimized firmware with **24+ hour battery life**!

### Next Steps:
1. **Touch the screen** - display should wake up
2. **Watch the serial output** - monitor battery status
3. **Try other examples** - Display, Sensors, Power, Wireless

## 🔧 Alternative Development Environments

### Arduino IDE Setup
1. Install **ESP32 Arduino Core 2.0.9+**
2. Install required libraries:
   - LilyGoLib
   - LovyanGFX  
   - LVGL
   - XPowersLib
   - SensorLib
3. Select **"LilyGo T-Watch S3"** board
4. Open example sketch and upload

### VS Code + PlatformIO
1. Install **PlatformIO IDE extension**
2. Open project folder
3. PlatformIO will auto-detect configuration
4. Use toolbar buttons to build/upload

## 📱 Essential Examples Overview

### 🚀 Try These Examples

#### BatteryOptimization (⭐ Recommended)
```bash
cp examples/essential/BatteryOptimization/* src/
pio run --target upload
```
- **Battery Life**: 24+ hours
- **Features**: Touch-activated display, auto-sleep
- **Best for**: Maximum battery efficiency

#### DisplayBasics
```bash
cp examples/essential/DisplayBasics/* src/
pio run --target upload
```
- **Battery Life**: 12-18 hours  
- **Features**: Touch coordinates, graphics, animations
- **Best for**: Learning display interaction

#### SensorBasics
```bash
cp examples/essential/SensorBasics/* src/
pio run --target upload
```
- **Battery Life**: 15-20 hours
- **Features**: Step counter, accelerometer, RTC
- **Best for**: Fitness tracking applications

#### PowerManagement
```bash
cp examples/essential/PowerManagement/* src/
pio run --target upload
```
- **Battery Life**: 20+ hours
- **Features**: PMU control, sleep modes, wake-up sources
- **Best for**: Advanced power optimization

#### WirelessBasic
```bash
cp examples/essential/WirelessBasic/* src/
pio run --target upload
```
- **Battery Life**: 8-12 hours
- **Features**: WiFi scanning, Bluetooth communication
- **Best for**: Connected applications

## 🔋 Battery Life Tips

### Maximum Battery (24+ hours)
```cpp
// From BatteryOptimization example
watch.setPowerProfile(POWER_PROFILE_LOW);  // 80MHz CPU
watch.setBrightness(30);                    // Minimal brightness
watch.powerIoctl(WATCH_POWER_RADIO, false); // Disable radio
watch.powerIoctl(WATCH_POWER_GPS, false);   // Disable GPS
```

### Balanced Usage (12-18 hours)
```cpp
// From other examples
watch.setPowerProfile(POWER_PROFILE_BALANCED); // 160MHz CPU
watch.setBrightness(100);                       // Medium brightness
```

### Performance Mode (8-12 hours)
```cpp
watch.setPowerProfile(POWER_PROFILE_HIGH); // 240MHz CPU
watch.setBrightness(255);                   // Maximum brightness
// All peripherals enabled
```

## 🐛 Common Issues & Solutions

### Upload Issues
**Problem**: `Could not open /dev/ttyACM1`  
**Solution**: 
1. Disconnect and reconnect USB cable
2. Try different USB port
3. Press reset button on watch

**Problem**: `Permission denied`  
**Solution**: 
```bash
# Add user to dialout group (Linux)
sudo usermod -a -G dialout $USER
# Log out and log back in
```

### Display Issues  
**Problem**: Display stays blank  
**Solution**: 
1. Check brightness: `watch.setBrightness(100);`
2. Verify power: `watch.powerIoctl(WATCH_POWER_DISPLAY_BL, true);`

**Problem**: Touch not working  
**Solution**: 
1. Check touch interrupt: `watch.getTouched()`
2. Restart watch: press power button 10 seconds

### Battery Issues
**Problem**: Battery drains quickly  
**Solution**: 
1. Use low power profile: `watch.setPowerProfile(POWER_PROFILE_LOW);`
2. Reduce brightness: `watch.setBrightness(30);`
3. Enable sleep: touch timeout after 15 seconds

**Problem**: Battery percentage wrong  
**Solution**: 
1. Calibrate: Full charge = 100%, 3.3V = 0%
2. Use voltage directly: `watch.getBattVoltage()`

## 📚 Next Steps

### 📖 Learn More
- [Full API Reference](api/LilyGoLib_API.md)
- [Hardware Documentation](hardware/TWatch_S3_Hardware.md)
- [Battery Optimization Guide](../BATTERY_OPTIMIZATION.md)

### 🛠️ Advanced Development
- Create custom watch faces
- Add sensor data logging
- Implement notification system
- Develop companion mobile app

### 🔧 Customize Examples
- Modify display timeouts
- Add custom sensor processing
- Create unique power profiles
- Implement data storage

## 🎯 Development Workflow

### Daily Development
```bash
# 1. Make changes to code
vim src/t-watch.ino

# 2. Build and test locally
pio run

# 3. Upload to watch
pio run --target upload --upload-port /dev/ttyACM1

# 4. Monitor serial output
pio device monitor -b 115200

# 5. Test battery optimization
# Let run for 1+ hour to see battery drain
```

### Production Build
```bash
# Create optimized production firmware
pio run -e twatch-s3-production

# Check binary size
ls .pio/build/twatch-s3-production/firmware.bin

# Should be ~800KB (vs 1.2MB default)
```

## 🎉 Success Checklist

✅ **Environment Setup**: PlatformIO installed and working  
✅ **Device Connected**: T-Watch S3 detected via USB  
✅ **First Upload**: Battery optimization flashed successfully  
✅ **Serial Monitor**: Can see debug output and status  
✅ **Touch Response**: Display wakes on touch interaction  
✅ **Battery Optimization**: Achieving extended battery life  

---

## 🚀 Ready for Development!

Your T-Watch S3 development environment is now optimized and ready for creating amazing smartwatch applications with **maximum battery life**!

**🎯 Start building your next smartwatch project today!**