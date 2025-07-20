#include "nexarustslab_font.h"

// Custom font renderer for NexaRustSlab bitmap font
class NexaRustSlabFontRenderer {
private:
    static const uint8_t* findCharData(uint16_t charCode) {
        const uint8_t* ptr = nexarustslab_font;
        while (pgm_read_byte(ptr) != 0) {
            if (pgm_read_byte(ptr) == charCode) {
                return ptr;
            }
            // Skip to next character
            uint8_t width = pgm_read_byte(ptr + 1);
            uint8_t height = pgm_read_byte(ptr + 2);
            uint8_t dataSize = (width * height + 7) / 8; // Calculate bitmap data size
            ptr += 6 + dataSize; // Skip header (6 bytes) + bitmap data
        }
        return nullptr;
    }

public:
    static void drawChar(lgfx::LGFXBase* display, int32_t x, int32_t y, uint16_t charCode, uint32_t color) {
        const uint8_t* charData = findCharData(charCode);
        if (!charData) return;

        uint8_t width = pgm_read_byte(charData + 1);
        uint8_t height = pgm_read_byte(charData + 2);
        uint8_t xOffset = pgm_read_byte(charData + 3);
        uint8_t yOffset = pgm_read_byte(charData + 4);
        uint8_t xAdvance = pgm_read_byte(charData + 5);
        
        const uint8_t* bitmap = charData + 6;
        
        // Draw bitmap
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                int byteIndex = (row * width + col) / 8;
                int bitIndex = (row * width + col) % 8;
                
                if (byteIndex < (width * height + 7) / 8) {
                    uint8_t byte = pgm_read_byte(bitmap + byteIndex);
                    if (byte & (0x80 >> bitIndex)) {
                        display->drawPixel(x + col + xOffset, y + row + yOffset, color);
                    }
                }
            }
        }
    }

    static uint8_t getCharWidth(uint16_t charCode) {
        const uint8_t* charData = findCharData(charCode);
        if (!charData) return 0;
        return pgm_read_byte(charData + 5); // xAdvance
    }

    static uint8_t getFontHeight() {
        return 50; // Height for NexaRustSlab font (50px)
    }
};

// Function to setup the NexaRustSlab font with custom renderer
void setupNexaRustSlabFontCustom(lgfx::LGFXBase& display) {
    // Use the custom NexaRustSlab font directly
    // No fallback - force use of the custom font
    Serial.println("Setting up NexaRustSlab custom font (50px)");
}

// Function to draw text using custom NexaRustSlab font
void drawNexaRustSlabText(lgfx::LGFXBase* display, int32_t x, int32_t y, const char* text, uint32_t color) {
    int32_t currentX = x;
    while (*text) {
        NexaRustSlabFontRenderer::drawChar(display, currentX, y, *text, color);
        currentX += NexaRustSlabFontRenderer::getCharWidth(*text);
        text++;
    }
} 