# Android Notification Sender App

## Overview
This document describes how to create a simple Android app that sends notifications to the T-Watch S3 via BLE without requiring additional apps on the phone.

## Method 1: Notification Access Service (Recommended)

### Android App Features:
- **Notification Listener Service**: Automatically captures all notifications
- **BLE Client**: Sends notifications to watch
- **Background Service**: Runs continuously
- **No User Interaction**: Works automatically

### Implementation Steps:

1. **Create Android Project**
```kotlin
// MainActivity.kt
class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        // Request notification access
        requestNotificationAccess()
        
        // Start BLE service
        startService(Intent(this, NotificationService::class.java))
    }
    
    private fun requestNotificationAccess() {
        val intent = Intent("android.settings.ACTION_NOTIFICATION_LISTENER_SETTINGS")
        startActivity(intent)
    }
}
```

2. **Notification Listener Service**
```kotlin
// NotificationService.kt
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

3. **BLE Manager**
```kotlin
// BleManager.kt
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
        // Write to T-Watch S3 BLE characteristic
        val characteristic = bluetoothGatt?.getService(UUID.fromString("0000fff0-0000-1000-8000-00805f9b34fb"))
            ?.getCharacteristic(UUID.fromString("0000fff1-0000-1000-8000-00805f9b34fb"))
        
        characteristic?.value = data
        bluetoothGatt?.writeCharacteristic(characteristic)
    }
}
```

## Method 2: Accessibility Service

### Alternative approach using Accessibility Service:
```kotlin
@RequiresApi(Build.VERSION_CODES.JELLY_BEAN)
class NotificationAccessibilityService : AccessibilityService() {
    override fun onAccessibilityEvent(event: AccessibilityEvent) {
        if (event.eventType == AccessibilityEvent.TYPE_NOTIFICATION_STATE_CHANGED) {
            val notification = event.parcelableData as? Notification
            if (notification != null) {
                val extras = notification.extras
                val app = event.packageName.toString()
                val title = extras.getString("android.title") ?: ""
                val message = extras.getString("android.text") ?: ""
                
                // Send via BLE
                BleManager(this).sendNotification(app, title, message)
            }
        }
    }
    
    override fun onInterrupt() {}
}
```

## Method 3: ADB Command (No App Required)

### For testing without installing an app:
```bash
# Enable notification access via ADB
adb shell settings put secure enabled_notification_listeners com.example.notificationsender/.NotificationService

# Send test notification
adb shell am broadcast -a android.intent.action.NOTIFICATION_POSTED \
  --es "app" "TestApp" \
  --es "title" "Test Title" \
  --es "message" "Test Message"
```

## Permissions Required

### AndroidManifest.xml:
```xml
<uses-permission android:name="android.permission.BLUETOOTH" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
<uses-permission android:name="android.permission.BLUETOOTH_SCAN" />

<service android:name=".NotificationService"
    android:permission="android.permission.BIND_NOTIFICATION_LISTENER_SERVICE"
    android:exported="false">
    <intent-filter>
        <action android:name="android.service.notification.NotificationListenerService" />
    </intent-filter>
</service>
```

## Installation Instructions

1. **Build the Android app** using Android Studio
2. **Install on phone** via ADB or APK
3. **Grant notification access** in Settings
4. **Enable BLE** and pair with T-Watch S3
5. **Test notifications** by receiving any notification on phone

## Features

- ✅ **Automatic**: No user interaction required
- ✅ **Real-time**: Instant notification forwarding
- ✅ **All apps**: Works with any app's notifications
- ✅ **Background**: Continues working when app is closed
- ✅ **Battery efficient**: Uses BLE for low power consumption

## Troubleshooting

1. **BLE not connecting**: Check if T-Watch S3 is advertising
2. **No notifications**: Verify notification access is granted
3. **Permission denied**: Check location and BLE permissions
4. **Service not starting**: Ensure app is not battery optimized

## Next Steps

1. **Compile and test** the Android app
2. **Upload notification_ble.ino** to T-Watch S3
3. **Pair devices** and test notification forwarding
4. **Customize UI** for better notification display 