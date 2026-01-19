#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "display_sharp.h"

SharpDisplay display;
unsigned long lastToggle = 0;
unsigned long testPhase = 0;

void setup() {
    Serial.begin(115200);
    delay(3000);
    
    Serial.println("\n========================================");
    Serial.println("GeekWatch - Display Test");
    Serial.println("========================================");
    
    Serial.print("Initializing display... ");
    if (display.begin()) {
        Serial.println("SUCCESS");
    } else {
        Serial.println("FAILED");
        while(1) delay(100);
    }
    
    Serial.println("\nTest sequence:");
    Serial.println("1. Clear screen (black)");
    Serial.println("2. Fill screen (white)");
    Serial.println("3. Draw test pattern");
    Serial.println("========================================\n");
    
    delay(1000);
}

void loop() {
    unsigned long now = millis();
    
    // Test sequence - change every 3 seconds
    unsigned long phase = (now / 3000) % 3;
    
    if (phase != testPhase) {
        testPhase = phase;
        
        switch (testPhase) {
            case 0:
                Serial.println("[0] Clearing screen (black)");
                display.clearDisplay();
                break;
            
            case 1:
                Serial.println("[1] Filling screen (white)");
                display.fillScreen(true);
                break;
            
            case 2:
                Serial.println("[2] Drawing test pattern");
                display.drawTestPattern();
                break;
        }
    }
    
    // Toggle VCOM at least once per second
    if (now - lastToggle >= 1000) {
        display.toggleVCOM();
        lastToggle = now;
        Serial.print(".");  // Heartbeat
    }
    
    delay(100);
}
