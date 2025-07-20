# Troubleshooting Guide

## Overview
Comprehensive troubleshooting guide for Edward Watch issues and solutions.

## Quick Diagnosis

### Check Serial Output
Always start by monitoring serial output for error messages:
```bash
pio device monitor
```

### Common Error Patterns
- **Compilation errors**: Library or configuration issues
- **Upload failures**: Hardware or connection problems
- **Runtime errors**: Configuration or API issues
- **Performance issues**: Power management or optimization problems

## Hardware Issues

### 1. Device Not Detected

#### Symptoms
- No COM port appears in device manager
- Upload fails with "port not found" error
- Device not recognized by computer

#### Solutions
1. **Check Power State**
   - Press crown button for 1 second to wake device
   - Ensure device is powered on
   - Check battery level

2. **Check USB Connection**
   - Try different USB cable
   - Test different USB ports
   - Check for loose connections

3. **Manual Boot Mode**
   - Remove back cover
   - Insert Micro-USB cable
   - Press and hold crown until USB device disappears
   - Press boot button (see image in main README)
   - Press crown for 1 second
   - Try upload again

### 2. Upload Failures

#### Symptoms
- Upload process starts but fails
- "Connection timeout" errors
- "Device busy" errors

#### Solutions
1. **Check Board Settings**
   ```ini
   # platformio.ini
   board = LilyGoWatch-S3
   upload_port = /dev/ttyACM0  # or COM port on Windows
   upload_speed = 921600
   ```

2. **Reset Device**
   - Disconnect USB
   - Wait 5 seconds
   - Reconnect USB
   - Try upload again

3. **Check Port Selection**
   - Verify correct port in PlatformIO
   - Try different port if available
   - Check device manager for port conflicts

### 3. Display Issues

#### Symptoms
- Screen not displaying
- Wrong colors or orientation
- Touch not working

#### Solutions
1. **Check Display Configuration**
   ```cpp
   // Verify display settings
   display.setRotation(2);  // Try different rotations
   display.setBrightness(255);
   ```

2. **Check Touch Configuration**
   ```cpp
   // Verify touch settings
   Wire1.begin(BOARD_TOUCH_SDA, BOARD_TOUCH_SCL);
   ```

3. **Check Power Domains**
   - Verify ALDO2 (backlight) is enabled
   - Verify ALDO3 (display) is enabled

## Software Issues

### 1. Compilation Errors

#### Common Errors and Solutions

**Library Not Found**
```
fatal error: TFT_eSPI.h: No such file or directory
```
**Solution**: Install required libraries
```bash
pio lib install "TFT_eSPI"
pio lib install "LovyanGFX"
```

**Conflicting Definitions**
```
warning: "COLOR_BACKGROUND" redefined
```
**Solution**: Check for duplicate definitions in multiple files

**ESP32 Version Issues**
```
esp_vad.h: No such file or directory
```
**Solution**: Use ESP32 board package version 2.0.9

### 2. Runtime Errors

#### Weather Issues

**Weather shows 0.0°C**
```
Weather temperature: 0.0°C
```
**Solutions**:
1. Check API key in `weather_config.ini`
2. Verify city name format (no quotes)
3. Check WiFi connection
4. Monitor serial for API errors

**Weather not updating**
```
Weather update not due yet. Next update in 224045 ms
```
**Solutions**:
1. Check `update_interval` setting
2. Verify first update was successful
3. Check WiFi connectivity
4. Monitor API response

**HTTP 400 Error**
```
HTTP response code: 400
Error response: <html><head><title>400 Bad Request</title></head>
```
**Solutions**:
1. Check city name encoding (spaces → %20)
2. Verify API key is valid
3. Check country code format
4. Ensure no quotes in city name

#### WiFi Issues

**WiFi not connecting**
```
WiFi connection failed!
```
**Solutions**:
1. Check SSID and password in `wifi_config.ini`
2. Verify network is in range
3. Check network security settings
4. Try different WiFi network

**WiFi disconnects frequently**
```
WiFi disconnected, attempting reconnection...
```
**Solutions**:
1. Adjust `WIFI_IDLE_TIMEOUT` setting
2. Check signal strength
3. Verify power management settings
4. Check for network interference

#### BLE Issues

**BLE not advertising**
```
BLE: Start advertising
```
**Solutions**:
1. Verify correct firmware is uploaded
2. Check BLE initialization
3. Monitor serial for BLE errors
4. Restart device

**Notifications not received**
```
No notifications received
```
**Solutions**:
1. Check Android app permissions
2. Verify notification access is granted
3. Check BLE pairing
4. Monitor serial for connection status

### 3. Performance Issues

#### Battery Drain

**Battery drains quickly**
**Solutions**:
1. Check sleep timeout settings
2. Verify WiFi power management
3. Monitor CPU frequency
4. Check for background processes

**Charging not detected**
**Solutions**:
1. Check USB connection
2. Verify PMU initialization
3. Check power domain settings
4. Monitor charging status

#### Memory Issues

**RAM usage high**
**Solutions**:
1. Optimize string usage
2. Reduce buffer sizes
3. Check for memory leaks
4. Monitor memory usage

**Flash usage high**
**Solutions**:
1. Remove unused libraries
2. Optimize code size
3. Check for duplicate code
4. Use compression if available

## Configuration Issues

### 1. Filesystem Problems

**Configuration not loaded**
```
Weather config file not found!
```
**Solutions**:
1. Verify `data/` folder exists
2. Check file names and extensions
3. Upload filesystem: `pio run -e twatch-s3 --target uploadfs`
4. Verify file contents

**Filesystem upload fails**
**Solutions**:
1. Check data folder structure
2. Verify file permissions
3. Try manual filesystem upload
4. Check for file size limits

### 2. API Issues

**OpenWeatherMap API errors**
**Solutions**:
1. Verify API key is valid
2. Check API usage limits
3. Verify city name format
4. Test API key manually

**Network connectivity issues**
**Solutions**:
1. Check WiFi configuration
2. Verify network connectivity
3. Test with different network
4. Check firewall settings

## Debugging Techniques

### 1. Serial Monitoring

**Enable Debug Output**
```cpp
Serial.begin(115200);
Serial.println("Debug: Starting application");
```

**Monitor Specific Functions**
```cpp
Serial.printf("Weather check: millis()=%lu, lastWeatherUpdate=%lu\n", 
             millis(), lastWeatherUpdate);
```

### 2. LED Indicators

**Use Built-in LED for Status**
```cpp
digitalWrite(LED_BUILTIN, HIGH);  // Error state
digitalWrite(LED_BUILTIN, LOW);   // Normal state
```

### 3. Memory Monitoring

**Check Memory Usage**
```cpp
Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
```

## Prevention

### 1. Best Practices

**Code Organization**
- Keep examples separate
- Use clear naming conventions
- Document configuration requirements
- Test thoroughly before committing

**Configuration Management**
- Use configuration files for settings
- Validate configuration on startup
- Provide clear error messages
- Include example configurations

**Error Handling**
- Implement proper error checking
- Provide meaningful error messages
- Include fallback mechanisms
- Log errors for debugging

### 2. Testing

**Before Deployment**
- Test all features thoroughly
- Verify configuration files
- Check power consumption
- Test error conditions

**Regular Maintenance**
- Monitor performance
- Check for updates
- Verify configurations
- Test connectivity

## Getting Help

### 1. Self-Help Resources
- [Setup Guide](../configuration/setup-guide.md)
- [Weather & WiFi Watch](../examples/weather-wifi-watch.md)
- [BLE Notifications](../examples/ble-notifications.md)
- [Main README](../../README.md)

### 2. Community Support
- [GitHub Issues](https://github.com/your-repo/edward-watch/issues)
- [Discussions](https://github.com/your-repo/edward-watch/discussions)
- [Documentation](https://github.com/your-repo/edward-watch/docs)

### 3. External Resources
- [LilyGO T-Watch S3](https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library)
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/)
- [PlatformIO Documentation](https://docs.platformio.org/)

### 4. Reporting Issues

**Include in Bug Reports**:
- Hardware version and configuration
- Software version and environment
- Complete error messages
- Steps to reproduce
- Serial output logs
- Configuration files (without sensitive data) 