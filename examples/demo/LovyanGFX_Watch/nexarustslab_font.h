#ifndef NEXARUSTSLAB_FONT_H
#define NEXARUSTSLAB_FONT_H

#include <LovyanGFX.hpp>

// NexaRustSlab Font Simulation
// Using FreeSansBold fonts which have a similar bold, industrial style

// Function to setup the NexaRustSlab font simulation
inline void setupNexaRustSlabFont(lgfx::LGFXBase& display) {
    // Use FreeSansBold24pt7b which has a bold, industrial style similar to NexaRustSlab
    display.setFont(&fonts::FreeSansBold24pt7b);
    Serial.println("Using FreeSansBold24pt7b to simulate NexaRustSlab bold style");
}

// Alternative function for smaller text
inline void setupNexaRustSlabFontSmall(lgfx::LGFXBase& display) {
    // Use FreeSansBold18pt7b for smaller text
    display.setFont(&fonts::FreeSansBold18pt7b);
    Serial.println("Using FreeSansBold18pt7b for smaller NexaRustSlab simulation");
}

// Function for very large text (like time display)
inline void setupNexaRustSlabFontLarge(lgfx::LGFXBase& display) {
    // Use FreeSansBold24pt7b with larger size
    display.setFont(&fonts::FreeSansBold24pt7b);
    display.setTextSize(1.5);  // Make it even larger
    Serial.println("Using FreeSansBold24pt7b with 1.5x size for large NexaRustSlab simulation");
}

#endif // NEXARUSTSLAB_FONT_H 