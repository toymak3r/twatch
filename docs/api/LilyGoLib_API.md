# 📚 LilyGoLib API Reference

Core API for the LilyGo T-Watch S3 smartwatch platform.

## 🚀 Quick Start

```cpp
#include <LilyGoLib.h>
#include <LV_Helper.h>

LilyGoLib watch;

void setup() {
    Serial.begin(115200);
    watch.begin();                    // Initialize hardware
    beginLvglHelper();              // Start graphics
    watch.setBrightness(100);       // Set display brightness
}

void loop() {
    // Handle touch input
    if (watch.getTouched()) {
        // Touch detected
    }
    
    lv_task_handler();  // Process UI
    delay(5);
}
```

## 📱 Core Initialization

### `begin()`
Initialize all T-Watch S3 hardware components.

```cpp
bool begin(Stream *stream = NULL);
```

**Parameters:**
- `stream` (optional): Serial output stream for debug messages

**Returns:** `true` if initialization successful, `false` otherwise

**Example:**
```cpp
if (!watch.begin(&Serial)) {
    Serial.println("Failed to initialize T-Watch");
    return;
}
```

### `beginLvglHelper()`
Initialize LVGL graphics library helper.

```cpp
void beginLvglHelper();
```

**Example:**
```cpp
watch.begin();
beginLvglHelper();  // Must call after watch.begin()
```

## 💡 Display Control

### `setBrightness()`
Control display backlight brightness (0-255).

```cpp
void setBrightness(uint8_t level);
uint8_t getBrightness();
```

**Parameters:**
- `level`: Brightness level (0 = off, 255 = maximum)

**Example:**
```cpp
watch.setBrightness(100);        // Medium brightness
watch.setBrightness(0);          // Display off (saves power)
```

### `getTouched()`
Check if touch screen is currently being touched.

```cpp
bool getTouched();
```

**Returns:** `true` if touch detected, `false` otherwise

**Example:**
```cpp
if (watch.getTouched()) {
    Serial.println("Touch detected!");
    // Handle touch interaction
}
```

### `getPoint()`
Get current touch coordinates.

```cpp
void getPoint(uint16_t *x, uint16_t *y);
```

**Parameters:**
- `x`: Pointer to store X coordinate
- `y`: Pointer to store Y coordinate

**Example:**
```cpp
uint16_t touchX, touchY;
if (watch.getTouched()) {
    watch.getPoint(&touchX, &touchY);
    Serial.printf("Touch at (%d, %d)", touchX, touchY);
}
```

## ⚡ Power Management

### Power Profiles
Configure global power settings for optimal battery life.

```cpp
void setPowerProfile(PowerProfile profile);
PowerProfile getPowerProfile();
```

**Profiles:**
- `POWER_PROFILE_LOW`: 80MHz CPU, minimal peripherals (24+ hours)
- `POWER_PROFILE_BALANCED`: 160MHz CPU, moderate features (12-18 hours)
- `POWER_PROFILE_HIGH`: 240MHz CPU, all features (8-12 hours)

**Example:**
```cpp
watch.setPowerProfile(POWER_PROFILE_LOW);  // Maximum battery life
```

### Individual Power Control
Fine-grained control over specific hardware components.

```cpp
void powerIoctl(enum PowerCtrlChannel ch, bool enable);
```

**Channels:**
- `WATCH_POWER_DISPLAY_BL`: Display backlight
- `WATCH_POWER_TOUCH_DISP`: Touch controller
- `WATCH_POWER_RADIO`: LoRa/WiFi radio module
- `WATCH_POWER_DRV2605`: Haptic feedback
- `WATCH_POWER_GPS`: GPS module

**Example:**
```cpp
watch.powerIoctl(WATCH_POWER_RADIO, false);  // Disable radio
watch.powerIoctl(WATCH_POWER_GPS, false);    // Disable GPS
```

### Direct Power Functions
Convenient functions for common power settings.

```cpp
void lowPower();      // 80MHz, minimal peripherals
void balancedPower();  // 160MHz, moderate features  
void highPower();     // 240MHz, all features
```

## 🔋 Battery & PMU

### Battery Monitoring
Get current battery status and voltage.

```cpp
uint16_t getBattVoltage();
bool isCharging();
bool isVbusIn();
```

**Returns:**
- `getBattVoltage()`: Battery voltage in millivolts (mV)
- `isCharging()`: `true` if currently charging
- `isVbusIn()`: `true` if USB connected

**Example:**
```cpp
uint16_t voltage = watch.getBattVoltage();
uint8_t percentage = map(voltage, 3300, 4200, 0, 100);

Serial.printf("Battery: %dmV (%d%%)", voltage, percentage);
```

### PMU Interrupts
Handle power management events (button press, charging, etc.).

```cpp
void attachPMU(void (*cb)(void));
uint64_t readPMU();
void clearPMU();
```

**Example:**
```cpp
void onPMUEvent() {
    uint64_t status = watch.readPMU();
    
    if (watch.isPekeyShortPressIrq()) {
        Serial.println("Power button pressed");
    }
    
    watch.clearPMU();  // Clear interrupt status
}

void setup() {
    watch.begin();
    watch.attachPMU(onPMUEvent);
}
```

## 📊 Sensor Functions

### Accelerometer (BMA423)
Access 3-axis accelerometer and step counting.

```cpp
uint16_t readBMA();
uint32_t getStepCounter();
void resetStepCounter();
float readAccelTemp();
```

**Example:**
```cpp
// Get step count
uint32_t steps = watch.getStepCounter();
Serial.printf("Steps today: %lu", steps);

// Get acceleration data
uint16_t irqStatus = watch.readBMA();
if (irqStatus & 0x80) {  // Step detected bit
    Serial.println("Step detected!");
}
```

### Real-Time Clock (PCF8563)
Access real-time clock and alarm functions.

```cpp
bool readRTC();
bool getDateTime(RTC_Date *date, RTC_Time *time);
```

**Example:**
```cpp
RTC_Date date;
RTC_Time time;

if (watch.getDateTime(&date, &time)) {
    Serial.printf("%04d/%02d/%02d %02d:%02d:%02d",
                 date.year, date.month, date.day,
                 time.hour, time.minute, time.second);
}
```

## 😴 Sleep Modes

### Sleep Configuration
Configure wake-up sources and enter sleep modes.

```cpp
void setSleepMode(SleepMode_t mode);
void sleep(uint32_t seconds = 0);
```

**Sleep Modes:**
- `PMU_BTN_WAKEUP`: Power button wake-up
- `ESP_TIMER_WAKEUP`: Timer-based wake-up
- `SENSOR_WAKEUP`: Accelerometer wake-up
- `RTC_WAKEUP`: RTC alarm wake-up
- `TOUCH_WAKEUP`: Touch screen wake-up

**Example:**
```cpp
// Sleep for 30 seconds, wake on power button
watch.setSleepMode(PMU_BTN_WAKEUP);
watch.sleep(30);

// Light sleep (fast wake-up)
esp_light_sleep_start();

// Deep sleep (maximum power saving)
esp_deep_sleep_start();
```

## 📡 Wireless Functions

### WiFi Integration
Basic WiFi connectivity with power optimization.

```cpp
// Note: WiFi functions use standard ESP32 WiFi library
// Example of power-optimized WiFi:
WiFi.setSleep(true);              // Enable power saving
WiFi.setTxPower(WIFI_POWER_8_5dBm); // Moderate transmission power
```

### Bluetooth
Serial Bluetooth communication.

```cpp
// Note: Uses standard ESP32 BluetoothSerial
BluetoothSerial SerialBT;
SerialBT.begin("T-Watch-S3");
```

## 🔧 Hardware Constants

### Pin Definitions (from `utilities.h`)
```cpp
#define BOARD_TFT_BL          45   // Display backlight
#define BOARD_TOUCH_INT        16   // Touch interrupt
#define BOARD_PMU_INT          21   // PMU interrupt  
#define BOARD_BMA423_INT1      14   // Accelerometer interrupt
#define BOARD_RTC_INT_PIN      17   // RTC interrupt
#define MOTOR_PIN             4    // Vibration motor
#define BOARD_IR_PIN          2    // IR transmitter
```

### Device Status Flags
```cpp
uint32_t getDeviceProbe();  // Returns bitmask of available hardware

// Flags:
#define WATCH_RADIO_ONLINE          _BV(0)
#define WATCH_TOUCH_ONLINE          _BV(1)
#define WATCH_DRV_ONLINE            _BV(2)
#define WATCH_PMU_ONLINE            _BV(3)
#define WATCH_RTC_ONLINE            _BV(4)
#define WATCH_BMA_ONLINE            _BV(5)
#define WATCH_GPS_ONLINE            _BV(6)
```

## 📈 Performance Tips

### Battery Optimization
```cpp
// Maximum battery life setup
watch.setPowerProfile(POWER_PROFILE_LOW);
watch.setBrightness(30);           // Minimal usable brightness
watch.powerIoctl(WATCH_POWER_RADIO, false);  // Disable unused peripherals
esp_light_sleep_start();            // Use light sleep
```

### Display Optimization
```cpp
// Reduce refresh rate for power saving
lv_disp_set refr_period(30);  // 30Hz instead of 60Hz

// Use partial updates when possible
lv_obj_invalidate(obj);  // Only update specific object
```

### Memory Optimization
```cpp
// Avoid string allocations in loops
// BAD:
for (int i = 0; i < 100; i++) {
    String msg = "Count: " + String(i);  // Creates new String each iteration
}

// GOOD:
char msg[20];
for (int i = 0; i < 100; i++) {
    snprintf(msg, sizeof(msg), "Count: %d", i);  // Reuses buffer
}
```

## 🐛 Error Handling

### Initialization Errors
```cpp
if (!watch.begin()) {
    // Check what failed
    uint32_t devices = watch.getDeviceProbe();
    
    if (!(devices & WATCH_PMU_ONLINE)) {
        Serial.println("PMU initialization failed");
    }
    if (!(devices & WATCH_BMA_ONLINE)) {
        Serial.println("Accelerometer not found");
    }
    // ... other checks
}
```

### Common Issues
1. **Display not working**: Check `getDeviceProbe()` for `WATCH_TOUCH_ONLINE`
2. **No touch response**: Verify `BOARD_TOUCH_INT` pin assignment
3. **Battery not reading**: Check PMU I2C connection
4. **Sleep not working**: Ensure wake-up source is configured

## 📚 Further Reading

- [Hardware Pinouts](../hardware/pinouts.md)
- [Battery Optimization Guide](../BATTERY_OPTIMIZATION.md)
- [Example Applications](../../examples/essential/)
- [Troubleshooting Guide](../guides/troubleshooting.md)

---

**🎯 This API provides everything needed for efficient T-Watch S3 development!**