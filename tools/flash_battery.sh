#!/bin/bash
# tools/flash_battery.sh - Build and flash the MAINFRAME firmware

echo "Flashing T-Watch S3 MAINFRAME firmware..."

# Check if device is connected
if ! ls /dev/ttyACM* >/dev/null 2>&1; then
    echo "No T-Watch S3 device found!"
    echo "Connect USB cable and try again"
    exit 1
fi

# Build and upload
echo "Building and uploading..."
python -m platformio run -e twatch-s3 --target upload --upload-port /dev/ttyACM1

if [ $? -eq 0 ]; then
    echo ""
    echo "MAINFRAME firmware flashed successfully!"
    echo "Press the physical button to wake the watch."
    echo "Edit src/config.h to change WiFi, timezone, brightness, and timeout."
    echo ""

    # Start serial monitor
    python -m platformio device monitor -b 115200 --port /dev/ttyACM1
else
    echo "Flash failed! Check device connection."
    exit 1
fi
