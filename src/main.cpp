#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "display_sharp.h"
#include "config.h"
#include <nrf_rtc.h>
#include <nrf_power.h>

SharpDisplay display;

// Button configuration
#define BUTTON_PIN 43  // P1.11 = 32 + 11 = 43
#define LONG_PRESS_MS 1000
#define DEBOUNCE_MS 50

// LED configuration (to disable status LED)
#define STATUS_LED_PIN 15  // P0.15 - Red LED on SuperMini board

// Button state
volatile bool buttonInterruptFlag = false;
bool buttonPressed = false;
unsigned long buttonPressTime = 0;
bool longPressHandled = false;
unsigned long lastDebounceTime = 0;
bool lastButtonState = HIGH;

// Display state
bool displayDirty = true;  // Flag to track if display needs update

// Clock state
uint8_t hours = 11;
uint8_t minutes = 37;
uint8_t seconds = 0;
bool isPM = false;  // AM
unsigned long lastClockUpdate = 0;

// Stopwatch state
uint16_t stopwatch1_millis = 0;  // milliseconds (0-999)
uint8_t stopwatch1_seconds = 0;
uint8_t stopwatch1_minutes = 0;
uint8_t stopwatch1_hours = 0;
uint16_t stopwatch2_millis = 0;
uint8_t stopwatch2_seconds = 0;
uint8_t stopwatch2_minutes = 0;
uint8_t stopwatch2_hours = 0;

bool stopwatch1_running = false;
bool stopwatch2_running = false;
unsigned long lastStopwatchUpdate = 0;

// Reset confirmation state
bool showResetConfirm = false;
unsigned long resetConfirmStartTime = 0;
#define RESET_CONFIRM_MS 3000

// 5x7 font for digits
const uint8_t font5x7[][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
};

void drawDigit(uint8_t x, uint8_t y, uint8_t digit, uint8_t scale = 1) {
    if (digit > 9) return;
    
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t colData = font5x7[digit][col];
        for (uint8_t row = 0; row < 7; row++) {
            if (colData & (1 << row)) {
                for (uint8_t sy = 0; sy < scale; sy++) {
                    for (uint8_t sx = 0; sx < scale; sx++) {
                        display.setPixel(x + col * scale + sx, y + row * scale + sy, false);
                    }
                }
            }
        }
    }
}

void drawChar(uint8_t x, uint8_t y, char c, uint8_t scale = 1) {
    // 5x7 font data
    const uint8_t charData[][5] = {
        {0x7E, 0x09, 0x09, 0x09, 0x7E}, // A
        {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
        {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
        {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
        {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
        {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
        {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
        {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
        {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
        {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
        {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
        {0x46, 0x49, 0x49, 0x49, 0x31}, // S
        {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
        {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
        {0x02, 0x01, 0x59, 0x09, 0x06}, // ? (question mark)
        {0x00, 0x00, 0x00, 0x00, 0x00}, // space
    };
    
    const uint8_t* data = nullptr;
    if (c == 'A') data = charData[0];
    else if (c == 'B') data = charData[1];
    else if (c == 'C') data = charData[2];
    else if (c == 'D') data = charData[3];
    else if (c == 'E') data = charData[4];
    else if (c == 'G') data = charData[5];
    else if (c == 'I') data = charData[6];
    else if (c == 'L') data = charData[7];
    else if (c == 'M') data = charData[8];
    else if (c == 'N') data = charData[9];
    else if (c == 'P') data = charData[10];
    else if (c == 'S') data = charData[11];
    else if (c == 'T') data = charData[12];
    else if (c == 'U') data = charData[13];
    else if (c == '?') data = charData[14];
    else if (c == ' ') data = charData[15];
    else return;
    
    if (!data) return;
    
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t colData = data[col];
        for (uint8_t row = 0; row < 7; row++) {
            if (colData & (1 << row)) {
                for (uint8_t sy = 0; sy < scale; sy++) {
                    for (uint8_t sx = 0; sx < scale; sx++) {
                        display.setPixel(x + col * scale + sx, y + row * scale + sy, false);
                    }
                }
            }
        }
    }
}

void drawColon(uint8_t x, uint8_t y, uint8_t scale = 1) {
    for (uint8_t sy = 0; sy < scale; sy++) {
        for (uint8_t sx = 0; sx < scale; sx++) {
            display.setPixel(x + sx, y + scale * 2 + sy, false);
            display.setPixel(x + sx, y + scale * 4 + sy, false);
        }
    }
}

void updateClock() {
    seconds++;
    displayDirty = true;  // Clock changed, need to redraw
    if (seconds >= 60) {
        seconds = 0;
        minutes++;
        if (minutes >= 60) {
            minutes = 0;
            hours++;
            if (hours == 12) {
                isPM = !isPM;  // Toggle AM/PM at 12:00
            }
            if (hours > 12) {
                hours = 1;
            }
        }
    }
}

void updateStopwatches() {
    unsigned long now = millis();
    unsigned long elapsed = now - lastStopwatchUpdate;
    lastStopwatchUpdate = now;
    
    if (stopwatch1_running) {
        stopwatch1_millis += elapsed;
        while (stopwatch1_millis >= 1000) {
            stopwatch1_millis -= 1000;
            stopwatch1_seconds++;
            displayDirty = true;  // Stopwatch changed
            if (stopwatch1_seconds >= 60) {
                stopwatch1_seconds = 0;
                stopwatch1_minutes++;
                if (stopwatch1_minutes >= 60) {
                    stopwatch1_minutes = 0;
                    stopwatch1_hours++;
                    if (stopwatch1_hours >= 100) {
                        stopwatch1_hours = 99;
                        stopwatch1_minutes = 59;
                        stopwatch1_seconds = 59;
                        stopwatch1_millis = 999;
                    }
                }
            }
        }
    }
    
    if (stopwatch2_running) {
        stopwatch2_millis += elapsed;
        while (stopwatch2_millis >= 1000) {
            stopwatch2_millis -= 1000;
            stopwatch2_seconds++;
            displayDirty = true;  // Stopwatch changed
            if (stopwatch2_seconds >= 60) {
                stopwatch2_seconds = 0;
                stopwatch2_minutes++;
                if (stopwatch2_minutes >= 60) {
                    stopwatch2_minutes = 0;
                    stopwatch2_hours++;
                    if (stopwatch2_hours >= 100) {
                        stopwatch2_hours = 99;
                        stopwatch2_minutes = 59;
                        stopwatch2_seconds = 59;
                        stopwatch2_millis = 999;
                    }
                }
            }
        }
    }
}

void resetStopwatches() {
    stopwatch1_millis = 0;
    stopwatch1_seconds = 0;
    stopwatch1_minutes = 0;
    stopwatch1_hours = 0;
    stopwatch2_millis = 0;
    stopwatch2_seconds = 0;
    stopwatch2_minutes = 0;
    stopwatch2_hours = 0;
    stopwatch1_running = false;
    stopwatch2_running = false;
    displayDirty = true;  // Display needs update
}

// Button interrupt handler
void buttonISR() {
    buttonInterruptFlag = true;
}

void handleButtonPress() {
    if (showResetConfirm) {
        // Button pressed during reset confirmation - do the reset
        resetStopwatches();
        showResetConfirm = false;
        #if DEBUG_SERIAL
        Serial.println("Stopwatches reset!");
        #endif
    } else {
        // Normal press - toggle between stopwatches
        if (!stopwatch1_running && !stopwatch2_running) {
            // First press - start stopwatch 1
            stopwatch1_running = true;
            #if DEBUG_SERIAL
            Serial.println("Stopwatch 1 started");
            #endif
        } else {
            // Toggle between stopwatches
            bool temp = stopwatch1_running;
            stopwatch1_running = stopwatch2_running;
            stopwatch2_running = temp;
            #if DEBUG_SERIAL
            Serial.print("Switched to stopwatch ");
            Serial.println(stopwatch1_running ? "1" : "2");
            #endif
        }
    }
    displayDirty = true;  // Button action requires display update
}

void handleLongPress() {
    if (!showResetConfirm) {
        showResetConfirm = true;
        resetConfirmStartTime = millis();
        displayDirty = true;  // Show confirmation dialog
        #if DEBUG_SERIAL
        Serial.println("Reset confirmation - press button within 3 seconds to confirm");
        #endif
    }
}

void updateButton() {
    // Only process if interrupt flag is set or button is currently pressed
    if (!buttonInterruptFlag && buttonPressed == HIGH) {
        return;
    }
    buttonInterruptFlag = false;
    
    bool reading = digitalRead(BUTTON_PIN);
    
    // Debounce
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
        if (reading != buttonPressed) {
            buttonPressed = reading;
            
            if (buttonPressed == LOW) {  // Button pressed (active low with pullup)
                buttonPressTime = millis();
                longPressHandled = false;
            } else {  // Button released
                unsigned long pressDuration = millis() - buttonPressTime;
                if (pressDuration < LONG_PRESS_MS && !longPressHandled) {
                    handleButtonPress();
                }
            }
        }
    }
    
    // Check for long press
    if (buttonPressed == LOW && !longPressHandled) {
        if (millis() - buttonPressTime >= LONG_PRESS_MS) {
            handleLongPress();
            longPressHandled = true;
        }
    }
    
    lastButtonState = reading;
}

void drawDisplay() {
    // Clear framebuffer (white background)
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH / 8; x++) {
            display.framebuffer[y][x] = 0xFF;
        }
    }
    
    // Draw clock in top left (HH:MM:SS AM/PM)
    uint8_t x = 2, y = 2;
    drawDigit(x, y, hours / 10, 1);
    drawDigit(x + 6, y, hours % 10, 1);
    drawColon(x + 12, y, 1);
    drawDigit(x + 14, y, minutes / 10, 1);
    drawDigit(x + 20, y, minutes % 10, 1);
    drawColon(x + 26, y, 1);
    drawDigit(x + 28, y, seconds / 10, 1);
    drawDigit(x + 34, y, seconds % 10, 1);
    // AM/PM
    drawChar(x + 41, y, isPM ? 'P' : 'A', 1);
    drawChar(x + 47, y, 'M', 1);
    
    // Draw stopwatches in center (bigger)
    uint8_t sw_y = 20;
    uint8_t scale = 2;
    
    // Stopwatch 1
    uint8_t sw1_x = 8;
    if (stopwatch1_running) drawChar(sw1_x - 6, sw_y, 'I', scale);  // Active indicator
    drawDigit(sw1_x, sw_y, stopwatch1_hours / 10, scale);
    drawDigit(sw1_x + 12, sw_y, stopwatch1_hours % 10, scale);
    drawColon(sw1_x + 24, sw_y, scale);
    drawDigit(sw1_x + 28, sw_y, stopwatch1_minutes / 10, scale);
    drawDigit(sw1_x + 40, sw_y, stopwatch1_minutes % 10, scale);
    drawColon(sw1_x + 52, sw_y, scale);
    drawDigit(sw1_x + 56, sw_y, stopwatch1_seconds / 10, scale);
    drawDigit(sw1_x + 68, sw_y, stopwatch1_seconds % 10, scale);
    drawChar(sw1_x + 82, sw_y, 'G', scale);  // Label
    
    // Stopwatch 2
    uint8_t sw2_x = 8;
    uint8_t sw2_y = sw_y + 18;
    if (stopwatch2_running) drawChar(sw2_x - 6, sw2_y, 'I', scale);  // Active indicator
    drawDigit(sw2_x, sw2_y, stopwatch2_hours / 10, scale);
    drawDigit(sw2_x + 12, sw2_y, stopwatch2_hours % 10, scale);
    drawColon(sw2_x + 24, sw2_y, scale);
    drawDigit(sw2_x + 28, sw2_y, stopwatch2_minutes / 10, scale);
    drawDigit(sw2_x + 40, sw2_y, stopwatch2_minutes % 10, scale);
    drawColon(sw2_x + 52, sw2_y, scale);
    drawDigit(sw2_x + 56, sw2_y, stopwatch2_seconds / 10, scale);
    drawDigit(sw2_x + 68, sw2_y, stopwatch2_seconds % 10, scale);
    drawChar(sw2_x + 82, sw2_y, 'L', scale);  // Label
    
    // Draw reset confirmation box if active
    if (showResetConfirm) {
        unsigned long timeLeft = RESET_CONFIRM_MS - (millis() - resetConfirmStartTime);
        if (timeLeft > RESET_CONFIRM_MS) timeLeft = 0;  // Handle overflow
        uint8_t secondsLeft = (timeLeft / 1000) + 1;
        
        // Draw centered box (148x30 pixels, centered on 160x68 screen)
        uint8_t box_x = 6;
        uint8_t box_y = 19;
        uint8_t box_w = 148;
        uint8_t box_h = 30;
        
        // Fill box interior with white
        for (uint8_t y = box_y + 1; y < box_y + box_h - 1; y++) {
            for (uint8_t x = box_x + 1; x < box_x + box_w - 1; x++) {
                display.setPixel(x, y, true);
            }
        }
        
        // Draw black border
        for (uint8_t x = box_x; x < box_x + box_w; x++) {
            display.setPixel(x, box_y, false);  // Top
            display.setPixel(x, box_y + box_h - 1, false);  // Bottom
        }
        for (uint8_t y = box_y; y < box_y + box_h; y++) {
            display.setPixel(box_x, y, false);  // Left
            display.setPixel(box_x + box_w - 1, y, false);  // Right
        }
        
        // Draw "SUBMIT DATA AND SLEEP?" on one line
        const char* message = "SUBMIT DATA AND SLEEP?";
        uint8_t text_x = box_x + 6;
        uint8_t text_y = box_y + 6;
        for (uint8_t i = 0; message[i] != '\0'; i++) {
            drawChar(text_x + i * 6, text_y, message[i], 1);
        }
        
        // Draw countdown centered below text
        uint8_t countdown_x = box_x + box_w / 2 - 3;
        drawDigit(countdown_x, box_y + 18, secondsLeft, 1);
    }
    
    display.refresh();
    displayDirty = false;  // Display is now up to date
}

void setup() {
    #if DEBUG_SERIAL
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("GeekWatch - Stopwatch Mode (Low Power)");
    Serial.println("========================================");
    #endif
    
    // Turn off the red status LED to save power
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);  // HIGH = off for active-low LED
    
    // Initialize button with interrupt
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, CHANGE);
    
    #if DEBUG_SERIAL
    Serial.print("Initializing display... ");
    #endif
    if (display.begin()) {
        #if DEBUG_SERIAL
        Serial.println("SUCCESS");
        #endif
    } else {
        #if DEBUG_SERIAL
        Serial.println("FAILED");
        #endif
        while(1) delay(100);
    }
    
    #if DEBUG_SERIAL
    Serial.println("Button on P1.11 (pin 43) - interrupt driven");
    Serial.println("Press: Switch stopwatch");
    Serial.println("Long press: Reset confirm");
    Serial.println("Power optimizations: ENABLED");
    Serial.println("========================================\n");
    #endif
    
    // Configure low power mode
    #if ENABLE_LOW_POWER_MODE
    // Enable DC/DC converter for better power efficiency
    sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    #endif
    
    lastStopwatchUpdate = millis();
    drawDisplay();
}

void loop() {
    unsigned long now = millis();
    
    // Update button state (only when interrupt fires or button held)
    updateButton();
    
    // Check reset confirmation timeout
    if (showResetConfirm && (now - resetConfirmStartTime >= RESET_CONFIRM_MS)) {
        showResetConfirm = false;
        displayDirty = true;
        #if DEBUG_SERIAL
        Serial.println("Reset cancelled");
        #endif
    }
    
    // Update clock every second
    if (now - lastClockUpdate >= 1000) {
        lastClockUpdate = now;
        updateClock();
    }
    
    // Update stopwatches - only update every second (not 10ms) to save power
    // The milliseconds aren't displayed anyway
    if (now - lastStopwatchUpdate >= 1000) {
        unsigned long elapsed = now - lastStopwatchUpdate;
        lastStopwatchUpdate = now;
        
        // Simplified update - just track seconds
        if (stopwatch1_running) {
            stopwatch1_seconds++;
            displayDirty = true;
            if (stopwatch1_seconds >= 60) {
                stopwatch1_seconds = 0;
                stopwatch1_minutes++;
                if (stopwatch1_minutes >= 60) {
                    stopwatch1_minutes = 0;
                    stopwatch1_hours++;
                    if (stopwatch1_hours >= 100) {
                        stopwatch1_hours = 99;
                        stopwatch1_minutes = 59;
                        stopwatch1_seconds = 59;
                    }
                }
            }
        }
        
        if (stopwatch2_running) {
            stopwatch2_seconds++;
            displayDirty = true;
            if (stopwatch2_seconds >= 60) {
                stopwatch2_seconds = 0;
                stopwatch2_minutes++;
                if (stopwatch2_minutes >= 60) {
                    stopwatch2_minutes = 0;
                    stopwatch2_hours++;
                    if (stopwatch2_hours >= 100) {
                        stopwatch2_hours = 99;
                        stopwatch2_minutes = 59;
                        stopwatch2_seconds = 59;
                    }
                }
            }
        }
    }
    
    // Only redraw display when something changed
    if (displayDirty) {
        drawDisplay();
    }
    
    // Sleep until next event (power optimization)
    #if ENABLE_LOW_POWER_MODE
    // Calculate time until next wakeup needed
    unsigned long nextUpdate = 1000;  // Default: wake every second for clock
    
    // If nothing is running and no confirmation dialog, can sleep longer
    if (!stopwatch1_running && !stopwatch2_running && !showResetConfirm) {
        nextUpdate = 1000;  // Just update clock every second
    }
    
    // Short delay for debouncing and power savings
    delay(50);  // 50ms sleep between checks
    #else
    delay(10);  // Minimal delay in non-low-power mode
    #endif
}
