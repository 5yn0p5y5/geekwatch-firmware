/**
 * @file display_simple.h
 * @brief Simplified display driver without Adafruit_GFX inheritance
 */

#ifndef DISPLAY_SIMPLE_H
#define DISPLAY_SIMPLE_H

#include <Arduino.h>
#include <SPI.h>
#include "config.h"

class GeekWatchDisplaySimple {
public:
    GeekWatchDisplaySimple(uint8_t clk, uint8_t mosi, uint8_t cs);
    ~GeekWatchDisplaySimple();
    
    bool begin();
    void clearDisplay();
    void display();
    void toggleVCOM();
    
    // Simple drawing functions
    void setPixel(int16_t x, int16_t y, bool color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, bool color);
    
private:
    uint8_t _clk, _mosi, _cs;
    uint8_t *_framebuffer;
    bool _vcom;
    SPIClass *_spi;
    
    size_t getFramebufferSize();
};

#endif // DISPLAY_SIMPLE_H
