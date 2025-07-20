#ifndef NEXARUSTSLAB_FONT_H
#define NEXARUSTSLAB_FONT_H

#include <LovyanGFX.hpp>

// Simple bitmap font data for NexaRustSlab simulation
// This is a simplified version that works with LovyanGFX

// Font metrics
#define NEXARUSTSLAB_FONT_WIDTH 18
#define NEXARUSTSLAB_FONT_HEIGHT 18

// Function to setup a font that simulates NexaRustSlab
inline void setupNexaRustSlabFont(lgfx::LGFXBase& display) {
    // Use a built-in font with larger size to simulate NexaRustSlab
    display.setFont(&fonts::Font2);  // Use Font2 which is larger
    Serial.println("Using Font2 to simulate NexaRustSlab");
}

#endif // NEXARUSTSLAB_FONT_H 