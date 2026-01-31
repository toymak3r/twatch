# T-Watch S3 Maximum Battery Life Guide

## Overview
This guide provides comprehensive strategies to maximize your T-Watch S3 battery life to achieve 24+ hours of operation on a single charge.

## Current Power Consumption Analysis

Based on the codebase analysis, the main power drains are:

1. **Display Backlight** (30-200mA depending on brightness)
2. **CPU Processing** (240MHz by default)
3. **Optional Peripherals** (GPS, Radio, Haptics)
4. **Touch Controller** (Continuous operation)
5. **Wireless Modules** (LoRa, WiFi, Bluetooth if enabled)

## Power Profiles Available

The T-Watch library provides three power profiles:

### LOW Power Profile (Recommended for Max Battery)
- **CPU Frequency**: 80MHz (67% power reduction)
- **Haptics**: Disabled
- **GPS**: Disabled  
- **Display**: User-controlled via `setBrightness(0)`
- **Estimated Battery Life**: 24-48 hours

### BALANCED Power Profile
- **CPU Frequency**: 160MHz
- **Haptics**: Disabled
- **GPS**: Disabled
- **Display**: User-controlled
- **Estimated Battery Life**: 18-24 hours

### HIGH Power Profile (Default)
- **CPU Frequency**: 240MHz
- **All Peripherals**: Enabled
- **Display**: User-controlled
- **Estimated Battery Life**: 8-12 hours

## Key Optimization Strategies

### 1. Display Management (Most Impactful)

```cpp
// Turn off display completely
watch.setBrightness(0);  // 0mA backlight
// Note: This also powers down the display controller

// Minimal brightness for visibility
watch.setBrightness(30); // ~5mA backlight

// Auto-timeout after inactivity
if (millis() - lastTouch > 15000) {
    watch.setBrightness(0);
}
```

### 2. CPU Frequency Scaling

```cpp
// Apply low power profile
watch.setPowerProfile(POWER_PROFILE_LOW);

// Or manually set CPU frequency
setCpuFrequencyMhz(80);  // From 240MHz default
```

### 3. Peripheral Management

```cpp
// Disable all non-essential peripherals
watch.powerIoctl(WATCH_POWER_DRV2605, false);  // Haptics
watch.powerIoctl(WATCH_POWER_GPS, false);      // GPS
watch.powerIoctl(WATCH_POWER_RADIO, false);    // LoRa radio
```

### 4. Sleep Modes

**Light Sleep** (Fast wake-up, moderate savings):
```cpp
esp_sleep_enable_timer_wakeup(5000000);  // 5 seconds
esp_light_sleep_start();
```

**Deep Sleep** (Maximum savings, slow wake-up):
```cpp
esp_sleep_enable_timer_wakeup(60000000);  // 1 minute
esp_deep_sleep_start();
```

### 5. Touch-Activated Display

```cpp
bool handleTouchWake() {
    if (watch.getTouched()) {
        watch.setBrightness(60);  // Medium brightness
        lastInteraction = millis();
        return true;
    }
    return false;
}
```

## Usage Scenarios and Battery Estimates

### **Watch Mode** (24+ hours)
- Display: Off, wakes on touch
- CPU: 80MHz
- Peripherals: All disabled
- Sleep: Light sleep between interactions
- **Result**: 24-48 hours

### **Active Use** (12-18 hours)
- Display: Medium brightness (60)
- CPU: 160MHz
- Peripherals: As needed
- Sleep: Timeout after 30 seconds
- **Result**: 12-18 hours

### **Heavy Use** (6-10 hours)
- Display: High brightness (150+)
- CPU: 240MHz
- Peripherals: GPS/Radio active
- Sleep: Minimal
- **Result**: 6-10 hours

## Implementation Examples

### 1. Load the Max Battery Life Example
```bash
# Upload to your T-Watch
cd examples/demo/MaxBatteryLife
pio run --target upload --upload-port /dev/ttyACM1
```

### 2. Load the Power Monitor
```bash
# Monitor consumption in real-time
cd examples/demo/PowerMonitor
pio run --target upload --upload-port /dev/ttyACM1
```

## Advanced Techniques

### Battery Voltage Monitoring
```cpp
uint16_t voltage = watch.getBattVoltage();
uint8_t percent = map(voltage, 3300, 4200, 0, 100);

// Critical battery handling
if (voltage < 3200) {
    enterLowBatterySleep();  // Deep sleep for 1 hour
}
```

### Temperature Management
```cpp
float temp = watch.readCoreTemp();
if (temp > 60.0) {
    // Reduce performance to lower temperature
    watch.setPowerProfile(POWER_PROFILE_LOW);
}
```

### Interrupt-Driven Wake-up
```cpp
void setup() {
    // Configure touch interrupt for wake-up
    watch.attachBMA(touchWakeup);
    
    // Configure power button interrupt
    watch.attachPMU(powerButtonHandler);
}

void IRAM_ATTR touchWakeup() {
    // Wake from sleep on touch
}
```

## Testing and Validation

1. **Start with Power Monitor**: Load `PowerMonitor.ino` first to understand your current consumption
2. **Apply Max Battery Settings**: Use `MaxBatteryLife.ino` for maximum optimization
3. **Monitor Real Usage**: Track voltage drop over time to validate improvements
4. **Adjust for Your Use Case**: Modify timeouts and brightness based on your needs

## Expected Results

| Configuration | Display On Time | Total Battery Life |
|---------------|-----------------|-------------------|
| Default (High Profile) | 100% | 8-12 hours |
| Balanced + Auto-Timeout | 30% | 16-20 hours |
| Low + Touch Wake | 10% | 24-36 hours |
| Ultra Aggressive | 5% | 36-48 hours |

## Troubleshooting

### Battery Draining Too Fast
1. Check display brightness - biggest factor
2. Verify power profile is set to LOW
3. Ensure GPS/Radio are disabled
4. Check for stuck touch interrupt preventing sleep

### Watch Not Waking from Sleep
1. Verify touch interrupts are configured
2. Check PMU interrupt settings
3. Ensure timer wake-up is set as backup

### Display Issues
1. Call `watch.setBrightness()` after waking
2. Ensure display power rail (ALDO2) is enabled
3. Check LVGL task handler is running

## Integration Steps

1. **Upload and Test**: Start with the examples to validate improvements
2. **Customize for Your App**: Integrate power optimizations into your firmware
3. **Monitor Consumption**: Use Power Monitor to validate
4. **Fine-Tune**: Adjust timeouts and brightness for your use case

Following these strategies should easily achieve your goal of 24+ hours battery life, with potential for 48+ hours in aggressive power saving mode.