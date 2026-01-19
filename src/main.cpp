#include <Arduino.h>
#include "config.h"

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    
    // Wait up to 10 seconds for serial to connect
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime < 10000)) {
        delay(100);
    }
    
    delay(1000);  // Extra delay for stability
    
    Serial.println("\n\n\n");  // Send newlines to clear any garbage
    Serial.println("========================================");
    Serial.println("GeekWatch Firmware - Serial Test");
    Serial.println("========================================");
    Serial.print("FW: ");
    Serial.println(FIRMWARE_VERSION);
    Serial.print("HW: ");
    Serial.println(HARDWARE_VERSION);
    Serial.print("Millis: ");
    Serial.println(millis());
    Serial.println("\nIf you see this, serial works!");
    Serial.println("========================================\n");
}

void loop() {
    static unsigned long lastPrint = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastPrint >= 1000) {
        Serial.print("[");
        Serial.print(currentTime / 1000);
        Serial.println("s] Running");
        lastPrint = currentTime;
    }

    delay(10);
}
