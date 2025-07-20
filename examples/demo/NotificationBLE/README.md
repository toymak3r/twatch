# T-Watch S3 - BLE Notifications Example

## Overview
This example demonstrates how to receive smartphone notifications via BLE on the T-Watch S3 without requiring additional apps on the phone.

## Files
- `notification_ble.ino` - Main firmware for BLE notifications
- `notification_sender.apk` - Android app documentation
- `README_NOTIFICATIONS.md` - Complete implementation guide

## Quick Start

### 1. Upload Firmware
```bash
cd examples/demo/NotificationBLE/
pio run -e twatch-s3 --target upload -f notification_ble.ino
```

### 2. Create Android App
Follow the instructions in `notification_sender.apk` to create the Android notification forwarding app.

### 3. Test
1. Install the Android app
2. Grant notification access
3. Pair with T-Watch S3
4. Receive notifications on the watch

## Features
- ✅ BLE GATT Server for notification reception
- ✅ Notification display with app, title, message
- ✅ Touch navigation between notifications
- ✅ Notification history (up to 10 notifications)
- ✅ Long press to clear notifications

## BLE Protocol
- **Service UUID**: `0000fff0-0000-1000-8000-00805f9b34fb`
- **Characteristic**: `0000fff1-0000-1000-8000-00805f9b34fb`
- **Format**: JSON with app, title, message, timestamp

## Usage
- **Tap**: Navigate to next notification
- **Long Press (2s)**: Clear all notifications
- **Auto-display**: Shows notification counter and preview

For complete documentation, see `README_NOTIFICATIONS.md`. 