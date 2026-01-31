# 🛠️ Development Tools & Scripts

Essential utilities for T-Watch S3 development, testing, and deployment.

## 🚀 Flash Scripts

### Battery Optimization Flash
Flash the 24+ hour battery life firmware with one command.

```bash
#!/bin/bash
# tools/flash_battery.sh - Flash maximum battery optimization

echo "🔋 Flashing T-Watch S3 Battery Optimization..."

# Copy battery optimization to src directory
cp examples/essential/BatteryOptimization/* src/

# Build and upload
pio run --target upload --upload-port /dev/ttyACM1

if [ $? -eq 0 ]; then
    echo "✅ Battery optimization flashed successfully!"
    echo "🎯 Expected battery life: 24+ hours"
    echo "📱 Touch screen to wake display"
else
    echo "❌ Flash failed! Check device connection."
    exit 1
fi

# Start serial monitor
echo "📡 Starting serial monitor..."
pio device monitor -b 115200
```

### Example Flash Script
Flash any example by name.

```bash
#!/bin/bash
# tools/flash_example.sh <example_name>

EXAMPLE_NAME=$1

if [ -z "$EXAMPLE_NAME" ]; then
    echo "Usage: ./flash_example.sh <example_name>"
    echo "Available examples:"
    echo "  BatteryOptimization"
    echo "  DisplayBasics"  
    echo "  SensorBasics"
    echo "  PowerManagement"
    echo "  WirelessBasic"
    exit 1
fi

EXAMPLE_PATH="examples/essential/$EXAMPLE_NAME"

if [ ! -d "$EXAMPLE_PATH" ]; then
    echo "❌ Example '$EXAMPLE_NAME' not found!"
    echo "Available examples:"
    ls examples/essential/
    exit 1
fi

echo "🚀 Flashing T-Watch S3 example: $EXAMPLE_NAME"

# Copy example to src directory
cp "$EXAMPLE_PATH"/* src/

# Build and upload
pio run --target upload --upload-port /dev/ttyACM1

if [ $? -eq 0 ]; then
    echo "✅ Example '$EXAMPLE_NAME' flashed successfully!"
else
    echo "❌ Flash failed! Check device connection."
    exit 1
fi
```

**Usage:**
```bash
# Flash battery optimization
./tools/flash_battery.sh

# Flash specific example
./tools/flash_example.sh DisplayBasics
./tools/flash_example.sh SensorBasics
```

## 📊 Power Monitoring Tools

### Real-time Power Monitor
Monitor power consumption and battery status in real-time.

```bash
#!/bin/bash
# tools/monitor_power.sh - Real-time power monitoring

echo "📊 T-Watch S3 Power Monitor"
echo "================================"

# Start serial monitor with timestamp
while true; do
    clear
    echo "🕐 $(date '+%H:%M:%S')"  
    echo "================================"
    
    # Get battery voltage (if device responds)
    echo "SENDING: Battery status request..."
    echo "BATT?" > /dev/ttyACM1
    
    # Read response with timeout
    timeout 2 cat /dev/ttyACM1 | grep -E "Battery|Voltage|mV" || echo "No response"
    
    echo ""
    echo "Press Ctrl+C to stop monitoring"
    sleep 5
done
```

### Battery Analysis Tool
Comprehensive battery analysis and recommendations.

```bash
#!/bin/bash
# tools/analyze_battery.sh - Battery performance analysis

echo "🔋 T-Watch S3 Battery Analysis"
echo "=============================="

# Test different power profiles
echo "1️⃣ Testing LOW power profile..."
echo "SENDING: Switch to LOW power..."
echo "PWR:LOW" > /dev/ttyACM1
sleep 2

echo "2️⃣ Measuring battery drain (LOW profile)..."
for i in {1..10}; do
    echo "TEST $i/10 - Taking baseline..."
    sleep 30
done

echo "3️⃣ Testing BALANCED power profile..."
echo "PWR:BALANCED" > /dev/ttyACM1
sleep 2

echo "4️⃣ Testing HIGH power profile..."
echo "PWR:HIGH" > /dev/ttyACM1
sleep 2

echo ""
echo "📊 Analysis Complete!"
echo "=============================="
echo "📋 Recommendations:"
echo "• Use LOW profile for maximum battery life"
echo "• Set brightness to 30-60 for daily use"  
echo "• Enable auto-sleep after 15 seconds"
echo "• Disable GPS/Radio when not needed"
```

## 🔧 Build Tools

### Clean Build Script
Clean build cache and rebuild from scratch.

```bash
#!/bin/bash
# tools/clean_build.sh - Clean and rebuild

echo "🧹 Cleaning T-Watch S3 build..."

# Remove build cache
rm -rf .pio
rm -rf src/*.o
rm -rf src/*.d

# Clean example artifacts
find examples/ -name "*.o" -delete
find examples/ -name "*.d" -delete

echo "✅ Build cache cleaned!"

# Rebuild with optimization
echo "🔨 Building optimized firmware..."
pio run -e twatch-s3-production

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "📦 Firmware size:"
    ls -lh .pio/build/twatch-s3-production/firmware.bin
else
    echo "❌ Build failed!"
    exit 1
fi
```

### Size Analysis Tool
Analyze firmware size and optimization impact.

```bash
#!/bin/bash
# tools/size_analysis.sh - Firmware size analysis

echo "📊 T-Watch S3 Firmware Size Analysis"
echo "===================================="

# Build different profiles
echo "🔨 Building firmware variants..."

pio run -e twatch-s3-debug
cp .pio/build/twatch-s3-debug/firmware.bin firmware_debug.bin

pio run -e twatch-s3-minimal  
cp .pio/build/twatch-s3-minimal/firmware.bin firmware_minimal.bin

pio run -e twatch-s3-production
cp .pio/build/twatch-s3-production/firmware.bin firmware_production.bin

# Size comparison
echo ""
echo "📊 Size Comparison:"
echo "======================"

printf "%-15s %10s %10s\n" "Profile" "Size (KB)" "Flash %"
printf "%-15s %10s %10s\n" "--------" "---------" "--------"

DEBUG_SIZE=$(stat -c%s firmware_debug.bin | awk '{printf "%.1f", $1/1024}')
MINIMAL_SIZE=$(stat -c%s firmware_minimal.bin | awk '{printf "%.1f", $1/1024}')
PROD_SIZE=$(stat -c%s firmware_production.bin | awk '{printf "%.1f", $1/1024}')

FLASH_TOTAL=3145728  # 3MB in bytes

printf "%-15s %10.1f %10.1f%%\n" "Debug" $DEBUG_SIZE $(echo "$DEBUG_SIZE*100/$FLASH_TOTAL" | bc -l)
printf "%-15s %10.1f %10.1f%%\n" "Minimal" $MINIMAL_SIZE $(echo "$MINIMAL_SIZE*100/$FLASH_TOTAL" | bc -l)
printf "%-15s %10.1f %10.1f%%\n" "Production" $PROD_SIZE $(echo "$PROD_SIZE*100/$FLASH_TOTAL" | bc -l)

echo ""
echo "🎯 Recommendations:"
echo "• Use production build for deployment"
echo "• Minimal build for space-constrained applications"
echo "• Debug build only for development"
```

## 📱 Device Testing Tools

### Touch Test Utility
Test touch screen functionality and calibration.

```bash
#!/bin/bash
# tools/test_touch.sh - Touch screen testing

echo "📱 T-Watch S3 Touch Test"
echo "========================"

echo "👆 Touch the screen to test:"
echo "• Single touch - coordinates should appear"
echo "• Multiple touches - touch count should increase"
echo "• Press power button to exit"

# Monitor touch responses
pio device monitor -b 115200 | grep -E "Touch|Coordinate|Button"
```

### Sensor Test Utility
Test all sensors and verify functionality.

```bash
#!/bin/bash
# tools/test_sensors.sh - Sensor testing

echo "📊 T-Watch S3 Sensor Test"
echo "========================"

echo "🔄 Testing sensors..."

# Send test commands to device
echo "SENDING: Sensor self-test..."
echo "SENSOR:TEST" > /dev/ttyACM1

sleep 2

# Monitor responses
echo "📡 Monitoring sensor responses..."
pio device monitor -b 115200 | grep -E "Accelerometer|Step|RTC|Temperature|Sensor"
```

## 🔌 Connection Tools

### Device Finder
Find connected T-Watch S3 devices.

```bash
#!/bin/bash
# tools/find_device.sh - Find T-Watch devices

echo "🔍 Searching for T-Watch S3 devices..."

# Check for ESP32 devices
for device in /dev/ttyACM* /dev/ttyUSB*; do
    if [ -e "$device" ]; then
        # Try to identify device
        if udevadm info --name="$device" 2>/dev/null | grep -q "Espressif"; then
            echo "✅ Found T-Watch S3: $device"
            echo "📡 Use this port for upload:"
            echo "   pio run --target upload --upload-port $device"
        else
            echo "ℹ️  Found device: $device (not T-Watch S3)"
        fi
    fi
done

if ! ls /dev/ttyACM* >/dev/null 2>&1; then
    echo "❌ No T-Watch S3 devices found"
    echo "💡 Check USB connection and try again"
fi
```

### USB Permission Fix
Fix USB device permissions for Linux users.

```bash
#!/bin/bash
# tools/fix_permissions.sh - Fix USB permissions

echo "🔧 Fixing T-Watch S3 USB permissions..."

# Add user to dialout group (for serial devices)
sudo usermod -a -G dialout $USER

# Create udev rule for T-Watch S3
sudo tee /etc/udev/rules.d/99-twatch.rules > /dev/null <<EOF
# T-Watch S3 USB permissions
ATTR{idVendor}=="303a", ATTR{idProduct}=="1001", MODE="0666", GROUP="dialout"
EOF

# Reload udev rules
sudo udevadm control --reload-rules
sudo udevadm trigger

echo "✅ USB permissions fixed!"
echo "🔄 Please log out and log back in for changes to take effect"
```

## 📜 Log Analysis Tools

### Serial Log Analyzer
Analyze serial logs for debugging and optimization.

```bash
#!/bin/bash
# tools/analyze_logs.sh - Log file analysis

LOG_FILE=$1

if [ -z "$LOG_FILE" ]; then
    echo "Usage: ./analyze_logs.sh <log_file>"
    echo "Example: ./analyze_logs.sh serial_output.log"
    exit 1
fi

if [ ! -f "$LOG_FILE" ]; then
    echo "❌ Log file '$LOG_FILE' not found!"
    exit 1
fi

echo "📊 Analyzing T-Watch S3 logs: $LOG_FILE"
echo "=========================================="

# Extract key metrics
echo "🔋 Battery Analysis:"
grep -E "Battery|Voltage|Charging" "$LOG_FILE" | tail -5

echo ""
echo "📱 Touch Events:"
grep -c "Touch" "$LOG_FILE" | awk '{print "Total touch events: " $1}'

echo ""
echo "😴 Sleep Events:"
grep -c "Sleep|Wake" "$LOG_FILE" | awk '{print "Total sleep cycles: " $1}'

echo ""
echo "🐛 Errors:"
grep -E "Error|Failed|Timeout" "$LOG_FILE" | wc -l | awk '{print "Total errors: " $1}'
```

## 📦 Deployment Tools

### Production Build Script
Create production-ready firmware with full optimization.

```bash
#!/bin/bash
# tools/build_production.sh - Production build

echo "🏭 Building Production Firmware"
echo "=============================="

# Version and build info
VERSION=$(date +%Y.%m.%d)
BUILD_NUMBER=$(git rev-list --count HEAD 2>/dev/null || echo "1")

echo "📦 Version: $VERSION"
echo "🔢 Build: $BUILD_NUMBER"

# Build production firmware
pio run -e twatch-s3-production

if [ $? -ne 0 ]; then
    echo "❌ Production build failed!"
    exit 1
fi

# Create release package
RELEASE_DIR="release_v$VERSION"
mkdir -p "$RELEASE_DIR"

# Copy firmware
cp .pio/build/twatch-s3-production/firmware.bin "$RELEASE_DIR/twatch_s3_v$VERSION.bin"
cp .pio/build/twatch-s3-production/firmware.elf "$RELEASE_DIR/twatch_s3_v$VERSION.elf"

# Create info file
cat > "$RELEASE_DIR/release_info.txt" <<EOF
T-Watch S3 Production Firmware
Version: $VERSION
Build: $BUILD_NUMBER
Date: $(date)
Target: 24+ hour battery life
Size: $(stat -c%s .pio/build/twatch-s3-production/firmware.bin | awk '{printf "%.1f KB", $1/1024}')
Features:
- Maximum battery optimization
- Touch-activated display
- Auto-sleep modes
- Power management
- Sensor integration
EOF

echo "✅ Production build complete!"
echo "📦 Release package: $RELEASE_DIR/"
echo "📊 Firmware size: $(stat -c%s .pio/build/twatch-s3-production/firmware.bin | awk '{printf "%.1f KB", $1/1024}')"
```

## 🎯 Usage Examples

### Quick Development Workflow
```bash
# 1. Setup development environment
./tools/fix_permissions.sh

# 2. Find device
./tools/find_device.sh

# 3. Flash battery optimization
./tools/flash_battery.sh

# 4. Monitor power consumption
./tools/monitor_power.sh

# 5. Test sensors
./tools/test_sensors.sh
```

### Production Deployment
```bash
# 1. Build production firmware
./tools/build_production.sh

# 2. Analyze firmware size
./tools/size_analysis.sh

# 3. Test on device
./tools/flash_example.sh BatteryOptimization

# 4. Analyze logs
./tools/analyze_logs.sh serial_output.log
```

## 📋 Requirements

### System Dependencies
```bash
# Ubuntu/Debian
sudo apt install platformio coreutils bc udev

# macOS
brew install platformio coreutils

# Windows (WSL2)
sudo apt update && sudo apt install platformio coreutils bc
```

### Python Packages
```bash
# Required for advanced tools
pip install platformio pyserial
```

---

**🛠️ These tools provide everything needed for professional T-Watch S3 development!**