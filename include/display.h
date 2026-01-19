/**
 * @file display.h
 * @brief Display driver for LS011B7DH03 Sharp Memory Display
 * 
 * This display uses a modified Sharp Memory Display protocol.
 * Resolution: 160x68 pixels, monochrome (1-bit per pixel)
 * Interface: SPI
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include "config.h"

class GeekWatchDisplay : public Adafruit_GFX {
public:
    /**
     * @brief Constructor
     * @param clk SPI clock pin
     * @param mosi SPI MOSI pin
     * @param cs Chip select pin
     */
    GeekWatchDisplay(uint8_t clk, uint8_t mosi, uint8_t cs);

    /**
     * @brief Initialize the display
     * @return true if successful, false otherwise
     */
    bool begin();

    /**
     * @brief Draw a single pixel
     * @param x X coordinate
     * @param y Y coordinate
     * @param color Color (0=white, 1=black)
     */
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

    /**
     * @brief Send framebuffer to display
     */
    void display();

    /**
     * @brief Clear the framebuffer
     */
    void clearDisplay();

    /**
     * @brief Toggle VCOM bit (must be called periodically)
     * Sharp memory displays require periodic VCOM toggling
     */
    void toggleVCOM();

    /**
     * @brief Set display brightness (if supported)
     * @param brightness 0-255
     */
    void setBrightness(uint8_t brightness);

private:
    uint8_t _clk, _mosi, _cs;
    uint8_t *_framebuffer;  // 1-bit per pixel framebuffer
    bool _vcom;             // VCOM state
    SPIClass *_spi;

    /**
     * @brief Send a command to the display
     * @param line Line number to update (or 0 for clear)
     * @param data Line data
     * @param len Data length
     */
    void sendCommand(uint8_t cmd, uint8_t *data, size_t len);

    /**
     * @brief Calculate framebuffer size in bytes
     */
    size_t getFramebufferSize();
};

#endif // DISPLAY_H
