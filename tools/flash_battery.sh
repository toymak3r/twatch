#!/bin/bash
# tools/flash_battery.sh - Flash maximum battery optimization

echo "🔋 Flashing T-Watch S3 Battery Optimization..."

# Check if device is connected
if ! ls /dev/ttyACM* >/dev/null 2>&1; then
    echo "❌ No T-Watch S3 device found!"
    echo "💡 Connect USB cable and try again"
    exit 1
fi

# Copy battery optimization to src directory
echo "📁 Copying BatteryOptimization example..."
cp examples/essential/BatteryOptimization/* src/

# Build and upload
echo "🔨 Building and uploading..."
pio run --target upload --upload-port /dev/ttyACM1

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Battery optimization flashed successfully!"
    echo "🎯 Expected battery life: 24+ hours"
    echo "📱 Touch screen to wake display"
    echo "📡 Starting serial monitor..."
    echo ""
    echo "💡 Quick Tips:"
    echo "• Touch screen to wake display"
    echo "• Display auto-sleeps after 15 seconds"  
    echo "• Tap to cycle brightness (30→60→120→200)"
    echo "• Monitor battery via serial output"
    echo ""
    
    # Start serial monitor
    pio device monitor -b 115200
else
    echo "❌ Flash failed! Check device connection."
    exit 1
fi