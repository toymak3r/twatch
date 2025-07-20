# Hardware Schematics

## Overview
This directory contains hardware schematics and circuit diagrams for the LilyGO T-Watch series.

## Schematics

### T-Watch S3
- **[T_WATCH_S3.pdf](T_WATCH_S3.pdf)** - Complete schematic for T-Watch S3 (906KB)
  - ESP32-S3 microcontroller
  - Display and touch interface
  - Power management (AXP2101)
  - Sensors and peripherals
  - Radio modules (SX1262/SX1280/CC1101)

### T-Watch 2020 Series
- **[T_WATCH-2020V01.pdf](T_WATCH-2020V01.pdf)** - T-Watch 2020 V01 schematic (157KB)
- **[T_WATCH-2020V02.pdf](T_WATCH-2020V02.pdf)** - T-Watch 2020 V02 schematic (202KB)
- **[T_WATCH-2020V03.pdf](T_WATCH-2020V03.pdf)** - T-Watch 2020 V03 schematic (220KB)

## Key Components

### Power Management
- **AXP2101**: Power management IC
- **Battery**: 502530 size (5x25x30mm)
- **Charging**: USB-C connector
- **Power Domains**: Multiple voltage rails

### Microcontroller
- **ESP32-S3**: Main processor
- **PSRAM**: External memory
- **Flash**: 16MB storage

### Display & Interface
- **ST7789**: Display controller
- **FT6336**: Touch controller
- **Backlight**: LED control

### Sensors
- **BMA423**: Accelerometer
- **Temperature**: On-board sensor
- **RTC**: Real-time clock

### Radio Modules
- **SX1262**: LoRa transceiver
- **SX1280**: High-speed radio
- **CC1101**: Sub-GHz radio

## Pin Definitions

### Power Pins
| Pin | Function | Voltage |
|-----|----------|---------|
| VBAT | Battery voltage | 3.0-4.2V |
| 3V3 | 3.3V rail | 3.3V |
| GND | Ground | 0V |

### Display Pins
| Pin | Function | Description |
|-----|----------|-------------|
| TFT_MOSI | SPI MOSI | Display data |
| TFT_SCLK | SPI clock | Display clock |
| TFT_CS | Chip select | Display select |
| TFT_DC | Data/Command | Display control |
| TFT_RST | Reset | Display reset |

### Touch Pins
| Pin | Function | Description |
|-----|----------|-------------|
| TOUCH_SDA | I2C data | Touch data |
| TOUCH_SCL | I2C clock | Touch clock |
| TOUCH_INT | Interrupt | Touch interrupt |

### Button Pins
| Pin | Function | Description |
|-----|----------|-------------|
| BUTTON1 | Crown button | Main input |
| BUTTON2 | Side button | Secondary input |

## Design Considerations

### Power Management
- Multiple power domains for different components
- Efficient power switching for battery life
- USB charging with proper current limiting
- Battery protection circuits

### Signal Integrity
- Proper grounding for analogue circuits
- EMI shielding for radio modules
- Signal routing to minimise interference
- Power supply decoupling

### Thermal Management
- Component placement for heat dissipation
- Thermal vias for heat transfer
- Adequate copper area for power components

## Troubleshooting

### Common Issues
- **Power problems**: Check AXP2101 connections
- **Display issues**: Verify SPI connections
- **Touch not working**: Check I2C connections
- **Radio problems**: Verify antenna connections

### Debug Points
- Test points for key signals
- Voltage measurement points
- Signal probe locations

## Related Documentation
- [Shell Design](../shell/) - 3D case files
- [Images](../images/) - Reference images
- [Main Documentation](../README.md) - Complete documentation

## Version History

### T-Watch S3
- Latest version with ESP32-S3
- Improved power management
- Enhanced radio options
- Better sensor integration

### T-Watch 2020 Series
- V01: Initial design
- V02: Improved layout
- V03: Final optimised version

## Contributing
When working with schematics:
1. Verify component values
2. Check pin assignments
3. Test power sequences
4. Document any modifications

---

*Last updated: [Current Date]*
*Schematic version: 1.0* 