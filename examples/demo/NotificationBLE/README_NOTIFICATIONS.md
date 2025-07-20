# T-Watch S3 - BLE Notifications System

## Overview
This system allows the T-Watch S3 to receive smartphone notifications via BLE without requiring additional apps on the phone. It uses Android's built-in notification access services.

## Features

### T-Watch S3 Side:
- ✅ **BLE GATT Server**: Receives notifications from phone
- ✅ **Notification Display**: Shows app, title, message, and timestamp
- ✅ **Notification History**: Stores up to 10 notifications
- ✅ **Touch Navigation**: Tap to cycle through notifications
- ✅ **Long Press**: Clear all notifications
- ✅ **Real-time Updates**: Instant notification display

### Android Side:
- ✅ **Notification Listener**: Captures all system notifications
- ✅ **BLE Client**: Sends notifications to watch
- ✅ **Background Service**: Works without user interaction
- ✅ **Automatic Operation**: No manual intervention needed

## Quick Start

### 1. Upload Firmware to T-Watch S3
```bash
# Navigate to the notification example
cd examples/demo/LovyanGFX_Watch/

# Upload the notification firmware
pio run -e twatch-s3 --target upload -f notification_ble.ino
```

### 2. Create Android App
Create a new Android project with the following components:

#### MainActivity.kt
```kotlin
class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        // Request notification access
        requestNotificationAccess()
        
        // Start BLE service
        startService(Intent(this, NotificationService::class.java))
        
        // Close app (service runs in background)
        finish()
    }
    
    private fun requestNotificationAccess() {
        val intent = Intent("android.settings.ACTION_NOTIFICATION_LISTENER_SETTINGS")
        startActivity(intent)
    }
}
```

#### NotificationService.kt
```kotlin
@RequiresApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
class NotificationService : NotificationListenerService() {
    private lateinit var bleManager: BleManager
    
    override fun onCreate() {
        super.onCreate()
        bleManager = BleManager(this)
    }
    
    override fun onNotificationPosted(sbn: StatusBarNotification) {
        val notification = sbn.notification
        val extras = notification.extras
        
        val app = sbn.packageName
        val title = extras.getString("android.title") ?: ""
        val message = extras.getString("android.text") ?: ""
        
        // Send via BLE
        bleManager.sendNotification(app, title, message)
    }
}
```

#### BleManager.kt
```kotlin
class BleManager(private val context: Context) {
    private var bluetoothGatt: BluetoothGatt? = null
    
    fun sendNotification(app: String, title: String, message: String) {
        val json = JSONObject().apply {
            put("app", app)
            put("title", title)
            put("message", message)
            put("timestamp", System.currentTimeMillis())
        }
        
        val data = json.toString().toByteArray()
        writeCharacteristic(data)
    }
    
    private fun writeCharacteristic(data: ByteArray) {
        val characteristic = bluetoothGatt?.getService(UUID.fromString("0000fff0-0000-1000-8000-00805f9b34fb"))
            ?.getCharacteristic(UUID.fromString("0000fff1-0000-1000-8000-00805f9b34fb"))
        
        characteristic?.value = data
        bluetoothGatt?.writeCharacteristic(characteristic)
    }
}
```

### 3. Install and Configure

1. **Build and install** the Android app
2. **Grant notification access** in Android Settings
3. **Enable BLE** and pair with T-Watch S3
4. **Test notifications** by receiving any notification

## BLE Protocol

### Service UUID
- **Service**: `0000fff0-0000-1000-8000-00805f9b34fb`
- **Characteristic**: `0000fff1-0000-1000-8000-00805f9b34fb`

### JSON Format
```json
{
  "app": "com.whatsapp",
  "title": "John Doe",
  "message": "Hello, how are you?",
  "timestamp": 1640995200000
}
```

## Usage

### T-Watch S3 Controls:
- **Tap**: Navigate to next notification
- **Long Press (2s)**: Clear all notifications
- **Auto-scroll**: Shows notification counter

### Display Information:
- **App Name**: Green text at top
- **Title**: White text in middle
- **Message**: Gray text below title
- **Timestamp**: Time since received
- **Connection Status**: BLE connected/disconnected

## Troubleshooting

### Common Issues:

1. **BLE Not Connecting**
   - Check if T-Watch S3 is advertising
   - Verify BLE is enabled on phone
   - Ensure devices are paired

2. **No Notifications Received**
   - Verify notification access is granted
   - Check if Android app is running
   - Ensure service is not battery optimized

3. **Permission Errors**
   - Grant location permission (required for BLE)
   - Grant notification access permission
   - Check BLE permissions

4. **Service Not Starting**
   - Disable battery optimization for the app
   - Ensure app has background permissions
   - Check if service is enabled in settings

### Debug Commands:

```bash
# Check BLE advertising
adb shell dumpsys bluetooth_manager

# Test notification sending
adb shell am broadcast -a android.intent.action.NOTIFICATION_POSTED \
  --es "app" "TestApp" \
  --es "title" "Test Title" \
  --es "message" "Test Message"

# Check notification access
adb shell settings get secure enabled_notification_listeners
```

## Advanced Features

### Customization Options:

1. **Notification Filtering**
   ```kotlin
   // Filter specific apps
   val allowedApps = listOf("com.whatsapp", "com.telegram", "com.skype")
   if (app in allowedApps) {
       bleManager.sendNotification(app, title, message)
   }
   ```

2. **Message Truncation**
   ```kotlin
   // Limit message length
   val maxLength = 100
   val truncatedMessage = if (message.length > maxLength) {
       message.substring(0, maxLength) + "..."
   } else {
       message
   }
   ```

3. **Priority Filtering**
   ```kotlin
   // Only high priority notifications
   if (notification.priority >= Notification.PRIORITY_HIGH) {
       bleManager.sendNotification(app, title, message)
   }
   ```

### Battery Optimization:

1. **BLE Power Management**
   - Use connection intervals
   - Implement sleep modes
   - Optimize data transmission

2. **Display Power Saving**
   - Auto-dim after notification
   - Sleep mode when not in use
   - Efficient refresh rates

## Security Considerations

1. **BLE Pairing**: Use secure pairing methods
2. **Data Encryption**: Consider encrypting notification data
3. **Access Control**: Limit which apps can send notifications
4. **Privacy**: Be mindful of sensitive notification content

## Future Enhancements

1. **Reply Functionality**: Send quick replies from watch
2. **Action Buttons**: Interact with notifications
3. **Rich Notifications**: Support images and media
4. **Multiple Devices**: Support multiple watches
5. **Cloud Sync**: Sync notifications across devices

## Support

For issues and questions:
1. Check the troubleshooting section
2. Review BLE connection logs
3. Verify Android permissions
4. Test with simple notifications first

## License

This project is open source and available under the MIT License. 