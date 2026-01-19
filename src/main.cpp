#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

unsigned long counter = 0;

void setup() {
    Serial.begin(115200);
    // SuperMini boards often need extra time for USB serial
    delay(3000);
    
    Serial.println("\n\n========================================");
    Serial.println("nRF52840 SuperMini - Serial Stream Test");
    Serial.println("========================================");
    Serial.println("Starting continuous data stream...\n");
}

void loop() {
    counter++;
    
    Serial.print("[");
    Serial.print(millis() / 1000);
    Serial.print("s] Message #");
    Serial.print(counter);
    Serial.print(" - millis: ");
    Serial.println(millis());
    
    delay(500);  // Send message every 500ms
}
