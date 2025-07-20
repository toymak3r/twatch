# 🎨 Custom Fonts for T-Watch S3 - NexaRustSlab Style

## 📋 Overview

This project implements a custom bitmap font system inspired by the NexaRustSlab font family, providing an industrial/bold style perfect for Pip-Boy themed interfaces.

## 🔤 Font Sources

### Free Fonts
- **dafont.com**: https://www.dafont.com/
- **Google Fonts**: https://fonts.google.com/
- **FontSpace**: https://www.fontspace.com/

### Recommended Industrial Fonts
- **Orbitron** (Google Fonts) - Futuristic, industrial
- **Audiowide** (Google Fonts) - Bold, tech-style
- **Chakra Petch** (Google Fonts) - Digital, industrial
- **Rajdhani** (Google Fonts) - Clean, modern

## 🛠️ Tools Required

### 1. Processing
- **Download**: https://processing.org/download
- **Purpose**: Convert TTF fonts to bitmap format
- **Version**: 4.x or later

### 2. File to Hex Converter
- **Online Tool**: https://tomeko.net/online_tools/file_to_hex.php
- **Purpose**: Convert binary files to C array format
- **Alternative**: Use `xxd` command line tool

### 3. Font Conversion Examples
- **VolosR Repository**: https://github.com/VolosR/CustomFonts/
- **Tutorial**: Follow the examples in the repository

## 📁 Project Structure

```
LovyanGFX_Watch/
├── nexarustslab_font.h          # Font header with bitmap data
├── nexarustslab_font.cpp        # Font renderer implementation
├── t-watch.ino                  # Main firmware file
└── README_CUSTOM_FONTS.md       # This documentation
```

## 🔧 Implementation Details

### Font Format (VolosR Style)
```cpp
// Character format: [char_code, width, height, x_offset, y_offset, x_advance, bitmap_data...]
const uint8_t nexarustslab_font[] PROGMEM = {
    0x30, 0x18, 0x20, 0x00, 0x00, 0x18,  // Character '0'
    0x00, 0x00, 0x1F, 0xF0, 0x3F, 0xF8,  // Bitmap data...
    // ... more bitmap data
};
```

### Font Renderer Class
```cpp
class NexaRustSlabFontRenderer {
    static void drawChar(lgfx::LGFXBase* display, int32_t x, int32_t y, uint16_t charCode, uint32_t color);
    static uint8_t getCharWidth(uint16_t charCode);
    static uint8_t getFontHeight();
};
```

## 🚀 Usage Examples

### 1. Basic Font Setup
```cpp
#include "nexarustslab_font.h"

void setup() {
    // Use built-in FreeSansBold fonts (fallback)
    setupNexaRustSlabFont(display);
    
    // Or use custom bitmap font
    setupNexaRustSlabFontCustom(display);
}
```

### 2. Text Rendering
```cpp
// Using built-in fonts
display.setTextColor(0xFF0000);  // Red color
display.println("12:34 PM");

// Using custom bitmap font
drawNexaRustSlabText(&display, 120, 120, "12:34 PM", 0xFF0000);
```

### 3. Different Font Sizes
```cpp
// Small text (FreeSansBold18pt7b)
setupNexaRustSlabFontSmall(display);

// Large text (FreeSansBold24pt7b + 1.5x)
setupNexaRustSlabFontLarge(display);
```

## 🎯 Current Implementation

### ✅ Implemented Characters
- **Numbers**: 0-9 (48-57)
- **Punctuation**: Colon (:)
- **Letters**: A, M, P (65, 77, 80)
- **Space**: (32)

### 🎨 Font Characteristics
- **Height**: 32 pixels
- **Baseline**: 28 pixels
- **Style**: Bold, industrial
- **Color**: Pure red (#FF0000) on black background

## 📝 Adding New Characters

### 1. Create Bitmap Data
```cpp
// Example: Adding character 'B'
0x42, 0x18, 0x20, 0x00, 0x00, 0x18,  // Character 'B'
0x7F, 0xFF, 0x7F, 0xFF, 0x60, 0x0C,  // Bitmap data...
// ... continue with 32 bytes of bitmap data
```

### 2. Update Font Array
Add the new character data to `nexarustslab_font[]` in `nexarustslab_font.h`.

### 3. Test Implementation
Compile and test the new character rendering.

## 🔄 Conversion Workflow

### Step 1: Choose Font
1. Visit dafont.com
2. Search for "industrial", "tech", "digital" fonts
3. Download TTF file

### Step 2: Convert with Processing
1. Open Processing
2. Load font conversion sketch
3. Import TTF file
4. Export as bitmap data

### Step 3: Convert to C Array
1. Use file to hex converter
2. Format as C array
3. Add to font header file

### Step 4: Test on Device
1. Compile firmware
2. Upload to T-Watch S3
3. Verify rendering

## 🎨 Color Scheme

### Pip-Boy Theme
- **Background**: Black (#000000)
- **Text**: Pure Red (#FF0000)
- **Accents**: Dark Gray (#333333)

### Custom Colors
```cpp
#define COLOR_BACKGROUND     0x000000
#define COLOR_CUSTOM_TEXT    0xFF0000
#define COLOR_ACCENT         0x333333
```

## 📱 T-Watch S3 Compatibility

### Hardware
- **Board**: LilyGo T-Watch S3
- **Display**: 240x240 AMOLED
- **Framework**: Arduino + LovyanGFX

### Performance
- **RAM Usage**: ~47KB (14.3%)
- **Flash Usage**: ~835KB (26.5%)
- **Font Rendering**: Optimized for speed

## 🛠️ Troubleshooting

### Common Issues

#### 1. Font Not Rendering
- Check character codes in font array
- Verify bitmap data format
- Ensure proper PROGMEM placement

#### 2. Memory Issues
- Reduce font height if needed
- Limit character set
- Use built-in fonts as fallback

#### 3. Performance Problems
- Optimize bitmap data
- Use smaller font sizes
- Implement character caching

### Debug Tips
```cpp
// Enable debug output
Serial.println("Font setup complete");

// Check character availability
if (findCharData('A')) {
    Serial.println("Character 'A' found");
}
```

## 🔗 Resources

### Documentation
- **LovyanGFX**: https://github.com/lovyan03/LovyanGFX
- **VolosR Fonts**: https://github.com/VolosR/CustomFonts/
- **PlatformIO**: https://platformio.org/

### Community
- **LilyGo Discord**: For T-Watch support
- **Arduino Forum**: For general questions
- **GitHub Issues**: For bug reports

## 📄 License

This font system is based on the VolosR CustomFonts project and follows the same licensing terms.

## ☕ Support

If you find this project helpful, consider supporting the original creators:
- **VolosR**: https://ko-fi.com/volosprojects
- **LilyGo**: https://www.lilygo.cc/

---

**Happy Coding! 🚀** 