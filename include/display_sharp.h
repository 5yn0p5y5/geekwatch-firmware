/**
 * @file display_sharp.h
 * @brief Simple driver for Sharp Memory Display LS011B7DH03
 * 
 * Protocol: SPI Mode 0, MSB first, up to 2MHz
 * Commands:
 * - 0x80: Write line(s)
 * - 0x20: VCOM toggle (must be done at least once per second)
 * - 0x04: Clear display
 */

#ifndef DISPLAY_SHARP_H
#define DISPLAY_SHARP_H

#include <Arduino.h>
#include <SPI.h>
#include "config.h"

class SharpDisplay {
public:
    SharpDisplay();
    
    bool begin();
    void clearDisplay();
    void setPixel(uint8_t x, uint8_t y, bool white);
    void drawLine(uint8_t y, const uint8_t* lineData);
    void refresh();
    void toggleVCOM();
    void clearFramebuffer();
    
    // Test patterns
    void fillScreen(bool white);
    void drawTestPattern();
    
    // Public framebuffer access
    uint8_t framebuffer[DISPLAY_HEIGHT][DISPLAY_WIDTH / 8];
    
private:
    bool vcomState;
    
    void sendCommand(uint8_t cmd);
    uint8_t reverseByte(uint8_t b);
};

#endif // DISPLAY_SHARP_H
