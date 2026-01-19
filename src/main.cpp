/**
 * @file main.cpp
 * @brief GeekWatch Firmware - Main Application
 */

#include <Arduino.h>
#include "config.h"
#include "display_simple.h"

GeekWatchDisplaySimple *display = nullptr;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("GeekWatch Firmware Starting...");
    Serial.println("========================================");
    Serial.print("Firmware Version: ");
    Serial.println(FIRMWARE_VERSION);
    Serial.print("Hardware Version: ");
    Serial.println(HARDWARE_VERSION);
    Serial.println();
    
    Serial.println("Testing simplified display driver (no Adafruit_GFX)");
    
    Serial.println("Creating display object...");
    display = new GeekWatchDisplaySimple(DISPLAY_SCK_PIN, DISPLAY_MOSI_PIN, DISPLAY_CS_PIN);
    Serial.println("  Display object created");
    
    Serial.println("Initializing display...");
    if (display->begin()) {
        Serial.println("  Display initialized successfully!");
        
        // Draw test pattern
        Serial.println("Drawing test pattern...");
        display->fillRect(10, 10, 50, 30, true);
        display->display();
        Serial.println("  Test pattern displayed");
    } else {
        Serial.println("  Display initialization failed");
    }
    
    Serial.println("\nSetup complete! Entering main loop...\n");
}

void loop() {
    static unsigned long lastPrint = 0;
    static unsigned long lastVCOM = 0;
    unsigned long currentTime = millis();

    // Toggle VCOM every second
    if (display && currentTime - lastVCOM >= 1000) {
        display->toggleVCOM();
        lastVCOM = currentTime;
    }

    // Print status every 5 seconds
    if (currentTime - lastPrint >= 5000) {
        Serial.print("Loop running... uptime: ");
        Serial.print(currentTime / 1000);
        Serial.println("s");
        lastPrint = currentTime;
    }

    delay(10);
}
