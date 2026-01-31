# 🔧 T-Watch S3 Hardware Reference

Complete hardware specifications and pinout reference for the LilyGo T-Watch S3 smartwatch.

## 📱 Hardware Overview

### Core Components
- **MCU**: ESP32-S3 (dual-core, 240MHz, 16MB Flash, 8MB PSRAM)
- **Display**: 1.3" ST7789 TFT LCD (240×240 pixels, 16-bit color)
- **Touch**: FT6X36 capacitive touch controller
- **PMU**: AXP2101 power management unit (7 configurable power rails)
- **Sensors**: BMA423 3-axis accelerometer, PCF8563 RTC
- **Audio**: MAX98357A I2S amplifier, PDM microphone
- **Wireless**: WiFi 802.11 b/g/n, Bluetooth 5.0
- **Storage**: FFat filesystem (9MB usable)

## 🔌 Pinout Diagram

```
T-Watch S3 Pinout (ESP32-S3)

Display & Touch:
┌─────────────────────────────────────┐
│ BOARD_TFT_MOSI      GPIO 13       │ SPI to ST7789
│ BOARD_TFT_SCLK      GPIO 18       │ SPI Clock  
│ BOARD_TFT_CS        GPIO 12       │ SPI Chip Select
│ BOARD_TFT_DC        GPIO 38       │ Data/Command
│ BOARD_TFT_BL        GPIO 45       │ Backlight PWM
│                                     │
│ BOARD_TOUCH_SDA     GPIO 39       │ I2C to FT6X36
│ BOARD_TOUCH_SCL     GPIO 40       │ I2C Clock
│ BOARD_TOUCH_INT     GPIO 16       │ Touch Interrupt
└─────────────────────────────────────┘

Sensors & Power:
┌─────────────────────────────────────┐
│ BOARD_I2C_SDA       GPIO 10       │ I2C to sensors
│ BOARD_I2C_SCL       GPIO 11       │ I2C Clock
│                                     │
│ BOARD_BMA423_INT1   GPIO 14       │ Accelerometer Int
│ BOARD_RTC_INT_PIN    GPIO 17       │ RTC Interrupt
│ BOARD_PMU_INT        GPIO 21       │ PMU Interrupt
│                                     │
│ MOTOR_PIN            GPIO 4        │ Vibration Motor
│ BOARD_IR_PIN         GPIO 2        │ IR Transmitter
└─────────────────────────────────────┘

Audio & Radio:
┌─────────────────────────────────────┐
│ BOARD_MIC_CLOCK      GPIO 44       │ PDM Mic Clock
│ BOARD_MIC_DATA       GPIO 47       │ PDM Mic Data
│                                     │
│ BOARD_DAC_IIS_BCK    GPIO 48       │ I2S Bit Clock
│ BOARD_DAC_IIS_WS     GPIO 15       │ I2S Word Select
│ BOARD_DAC_IIS_DOUT   GPIO 46       │ I2S Data Out
│                                     │
│ BOARD_RADIO_SCK      GPIO 3        │ SPI to LoRa
│ BOARD_RADIO_MISO     GPIO 4        │ SPI MISO
│ BOARD_RADIO_MOSI     GPIO 1        │ SPI MOSI
│ BOARD_RADIO_SS       GPIO 5        │ SPI Chip Select
│ BOARD_RADIO_DI01     GPIO 9        │ LoRa DIO1
│ BOARD_RADIO_RST      GPIO 8        │ LoRa Reset
│ BOARD_RADIO_BUSY     GPIO 7        │ LoRa Busy
└─────────────────────────────────────┘

GPS Shield (Optional):
┌─────────────────────────────────────┐
│ SHIELD_GPS_TX       GPIO 42       │ GPS TX
│ SHIELD_GPS_RX       GPIO 41       │ GPS RX
└─────────────────────────────────────┘

Programming & Debug:
┌─────────────────────────────────────┐
│ USB-C Programming Port              │ JTAG/Serial
│ ESP32-S3 Native USB              │ CDC/JTAG
│ GPIO 48, 47           D+/D-   │ USB Data Lines
└─────────────────────────────────────┘
```

## ⚡ Power Management (AXP2101)

### Power Rails Configuration
```cpp
// Power Rails and Functions
ALDO1:  RTC VBAT     (3.3V)  - Always on, real-time clock
ALDO2:  TFT Backlight (3.3V)  - Display brightness control
ALDO3:  Touch Control  (3.3V)  - FT6X36 touch controller
ALDO4:  Radio Module   (3.3V)  - LoRa/WiFi module
BLDO1:  Core Logic     (1.8V)  - ESP32-S3 core
BLDO2:  Haptics       (3.3V)  - DRV2605 vibration motor
BLDO3:  -              (3.3V)  - Not used
DCDC1:  Core Logic     (3.3V)  - ESP32-S3 main power
DCDC2:  -              (3.3V)  - Not used
DCDC3:  GPS Module     (3.3V)  - GPS module power
DCDC4:  -              (3.3V)  - Not used
DCDC5:  -              (2.5V)  - Not used
```

### Power Control Functions
```cpp
// Enable/disable power rails
watch.powerIoctl(WATCH_POWER_DISPLAY_BL, enable);   // ALDO2 - Backlight
watch.powerIoctl(WATCH_POWER_TOUCH_DISP, enable);  // ALDO3 - Touch
watch.powerIoctl(WATCH_POWER_RADIO, enable);       // ALDO4 - Radio
watch.powerIoctl(WATCH_POWER_DRV2605, enable);     // BLDO2 - Haptics
watch.powerIoctl(WATCH_POWER_GPS, enable);          // DCDC3 - GPS
```

### Battery Specifications
- **Battery Type**: 3.7V Li-ion (typically 300-500mAh)
- **Voltage Range**: 3.0V (empty) to 4.2V (full)
- **Charging Current**: 100-500mA (USB power dependent)
- **PMU Monitoring**: Real-time voltage and current measurement

## 📺 Display Specifications

### ST7789 TFT LCD
```cpp
Resolution:      240×240 pixels
Color Depth:      16-bit (RGB565)
Diagonal:        1.3 inches
Pixel Density:   ~263 PPI
Interface:       SPI (40MHz maximum)
Controller:      ST7789
Backlight:       White LED, PWM controlled
Touch:          FT6X36 capacitive controller
```

### Display Initialization
```cpp
// Display configuration (handled by LilyGoLib)
BOARD_TFT_MOSI:   GPIO 13    // SPI MOSI
BOARD_TFT_SCLK:   GPIO 18    // SPI Clock  
BOARD_TFT_CS:     GPIO 12    // SPI Chip Select
BOARD_TFT_DC:     GPIO 38    // Data/Command
BOARD_TFT_BL:     GPIO 45    // Backlight PWM
BOARD_TFT_RST:     -1         // Hardware Reset
```

### Touch Controller
```cpp
// FT6X36 Configuration
BOARD_TOUCH_SDA:   GPIO 39    // I2C Data
BOARD_TOUCH_SCL:   GPIO 40    // I2C Clock
BOARD_TOUCH_INT:   GPIO 16    // Touch Interrupt
I2C Address:      0x38       // Default FT6X36 address
```

## 📊 Sensor Specifications

### BMA423 Accelerometer
```cpp
Axes:            3-axis (X, Y, Z)
Range:            ±16g
Resolution:       16-bit
Data Rate:        12.5-1600 Hz
Features:         Step counter, tap detection, orientation
Interface:        I2C (0x19)
Interrupt:        GPIO 14 (configurable)
```

### Accelerometer Functions
```cpp
uint16_t irqStatus = watch.readBMA();           // Read interrupt status
uint32_t steps = watch.getStepCounter();        // Get step count
watch.resetStepCounter();                       // Reset step counter
float temp = watch.readAccelTemp();            // Die temperature
```

### PCF8563 Real-Time Clock
```cpp
Functions:        Time, Date, Alarm, Timer
Accuracy:         ±3 ppm (±0.1 seconds/day)
Backup:          CR1220 battery (when installed)
Interface:        I2C (0x51)
Interrupt:        GPIO 17 (configurable)
```

### RTC Functions
```cpp
RTC_Date date;
RTC_Time time;
watch.getDateTime(&date, &time);               // Get date/time
// Access individual fields:
// date.year, date.month, date.day, date.weekday
// time.hour, time.minute, time.second
```

## 🎵 Audio Specifications

### PDM Microphone
```cpp
Type:            Digital MEMS
Sample Rate:      16 kHz (fixed)
Resolution:      16-bit
Channels:        Mono
Interface:        PDM
Pins:            GPIO 44 (Clock), GPIO 47 (Data)
```

### I2S Audio Output
```cpp
Amplifier:       MAX98357A
Interface:       I2S
Sample Rate:      8-48 kHz
Resolution:      16/24/32-bit
Channels:        Stereo/Mono
Pins:            GPIO 48 (BCK), GPIO 15 (WS), GPIO 46 (DOUT)
```

### Haptic Feedback
```cpp
Controller:      DRV2605L
Interface:       I2C (0x5A)
Effects:         100+ built-in haptic effects
Power:          BLDO2 rail
Pin:            Motor control via GPIO 4
```

## 📡 Wireless Specifications

### WiFi (ESP32-S3)
```cpp
Standards:        802.11 b/g/n
Frequency:        2.4 GHz
Data Rate:        Up to 150 Mbps
Security:         WPA2/WPA3, WEP, Open
Power Management:   Deep sleep, modem sleep
Antenna:          On-board PCB antenna
```

### Bluetooth
```cpp
Version:          Bluetooth 5.0
Class:            Class 2
Range:            ~10 meters (typical)
Profiles:         SPP, BLE, HID
Antenna:          On-board PCB antenna
```

### LoRa Radio (Optional Shield)
```cpp
Chip:             SX1262/SX1280 (shield dependent)
Frequency Range:   150-960 MHz (depending on chip)
Data Rate:        0.018-1.376 Mbps
Range:            Several kilometers (open field)
SPI Interface:     GPIO pins configured for radio
```

## 🔧 Electrical Specifications

### Power Requirements
```cpp
Operating Voltage:   3.7V (battery) or 5V (USB)
Current Consumption:
  - Active (all on):   150-200mA
  - Normal (display):   80-120mA  
  - Low Power:          20-40mA
  - Deep Sleep:        <1mA
Charging:            USB-C, 5V/500mA max
```

### GPIO Specifications
```cpp
Number of GPIO:    48 total
Digital I/O:        43 pins
Analog Input:        20 pins (12-bit ADC)
PWM Output:         16 pins
Touch Sensing:       10 pins
I2C:               2 controllers
SPI:               3 controllers  
UART:               3 controllers
I2S:               1 controller
```

## 📋 Development Connectors

### USB-C Port
- **Function**: Programming, power, serial communication
- **Protocol**: USB 2.0 Full Speed
- **Power**: 5V/500mA (for charging)
- **Debug**: CDC Serial, JTAG (with appropriate firmware)

### Expansion Headers (if available)
- **GPIO Access**: Additional pins for custom hardware
- **Power**: Access to power rails (3.3V, 5V)
- **Communication**: Additional UART/I2C/SPI interfaces

## 🔍 Troubleshooting Hardware

### Display Issues
- **No display**: Check `BOARD_TFT_BL` power, verify SPI connections
- **Wrong colors**: Verify ST7789 initialization, check RGB565 format
- **Touch not working**: Check `BOARD_TOUCH_SDA/SCL` connections, verify I2C address 0x38

### Power Issues  
- **Not charging**: Check USB connection, verify VBUS detection
- **Short battery life**: Disable unused peripherals, enable sleep modes
- **PMU errors**: Verify I2C connections to AXP2101 (address 0x34)

### Sensor Issues
- **No accelerometer data**: Check BMA423 I2C connection (address 0x19)
- **RTC not keeping time**: Check backup battery, verify PCF8563 I2C (address 0x51)
- **Step counter not working**: Enable step detection interrupt, configure BMA423 properly

### Audio Issues
- **No microphone output**: Verify PDM clock/data connections, check sample rate
- **No speaker output**: Check I2S connections, verify MAX98357A enable
- **No haptic feedback**: Check DRV2605 I2C connection, verify BLDO2 power

## 📚 Related Documentation

- [LilyGoLib API Reference](api/LilyGoLib_API.md)
- [Battery Optimization Guide](../BATTERY_OPTIMIZATION.md)
- [Essential Examples](../../examples/essential/)
- [Troubleshooting Guide](guides/troubleshooting.md)

---

**🔧 This hardware reference provides everything needed for T-Watch S3 development!**