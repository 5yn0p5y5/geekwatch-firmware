/**
 * @file display.cpp
 * @brief Display driver implementation for LS011B7DH03
 */

#include "display.h"

// Sharp Memory Display commands
#define SHARP_CMD_WRITE_LINE    0x80
#define SHARP_CMD_VCOM          0x40
#define SHARP_CMD_CLEAR         0x20

GeekWatchDisplay::GeekWatchDisplay(uint8_t clk, uint8_t mosi, uint8_t cs) 
    : Adafruit_GFX(DISPLAY_WIDTH, DISPLAY_HEIGHT),
      _clk(clk),
      _mosi(mosi),
      _cs(cs),
      _vcom(false),
      _spi(nullptr) {
    
    // Allocate framebuffer (1 bit per pixel, rounded up to bytes)
    size_t fbSize = getFramebufferSize();
    _framebuffer = (uint8_t*)malloc(fbSize);
    if (_framebuffer) {
        memset(_framebuffer, 0, fbSize);
    }
}

bool GeekWatchDisplay::begin() {
    if (!_framebuffer) {
        return false;  // Memory allocation failed
    }

    // Configure CS pin
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, LOW);

    // Initialize SPI
    _spi = &SPI;
    _spi->begin();
    
    // Sharp Memory Displays typically use:
    // - Mode 0 (CPOL=0, CPHA=0)
    // - Max 2MHz clock
    // - MSB first
    _spi->beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

    delay(10);  // Allow display to stabilize

    // Clear display
    clearDisplay();
    display();

    return true;
}

void GeekWatchDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (!_framebuffer || x < 0 || y < 0 || x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }

    // Calculate byte and bit position in framebuffer
    // Each row is (DISPLAY_WIDTH / 8) bytes, rounded up
    uint16_t bytesPerRow = (DISPLAY_WIDTH + 7) / 8;
    uint16_t byteIndex = y * bytesPerRow + (x / 8);
    uint8_t bitMask = 1 << (7 - (x % 8));  // MSB first

    if (color) {
        _framebuffer[byteIndex] |= bitMask;   // Set pixel (black)
    } else {
        _framebuffer[byteIndex] &= ~bitMask;  // Clear pixel (white)
    }
}

void GeekWatchDisplay::display() {
    if (!_framebuffer || !_spi) {
        return;
    }

    digitalWrite(_cs, HIGH);
    delayMicroseconds(3);

    // Sharp Memory Display write protocol:
    // Byte 0: Command byte (write + VCOM)
    // For each line:
    //   - Line address (1-indexed)
    //   - Line data bytes
    //   - Dummy byte (0x00)
    // Final dummy byte

    uint8_t cmd = SHARP_CMD_WRITE_LINE;
    if (_vcom) {
        cmd |= SHARP_CMD_VCOM;
    }

    _spi->transfer(cmd);

    uint16_t bytesPerRow = (DISPLAY_WIDTH + 7) / 8;
    
    for (uint16_t line = 0; line < DISPLAY_HEIGHT; line++) {
        // Send line address (1-indexed)
        _spi->transfer(line + 1);
        
        // Send line data
        for (uint16_t b = 0; b < bytesPerRow; b++) {
            _spi->transfer(_framebuffer[line * bytesPerRow + b]);
        }
        
        // Send dummy byte
        _spi->transfer(0x00);
    }

    // Final dummy byte
    _spi->transfer(0x00);

    delayMicroseconds(3);
    digitalWrite(_cs, LOW);
}

void GeekWatchDisplay::clearDisplay() {
    if (_framebuffer) {
        memset(_framebuffer, 0, getFramebufferSize());
    }
}

void GeekWatchDisplay::toggleVCOM() {
    if (!_spi) {
        return;
    }

    _vcom = !_vcom;

    // Send VCOM toggle command
    digitalWrite(_cs, HIGH);
    delayMicroseconds(3);
    
    uint8_t cmd = _vcom ? SHARP_CMD_VCOM : 0x00;
    _spi->transfer(cmd);
    _spi->transfer(0x00);  // Dummy byte
    
    delayMicroseconds(3);
    digitalWrite(_cs, LOW);
}

void GeekWatchDisplay::setBrightness(uint8_t brightness) {
    // LS011B7DH03 doesn't have hardware brightness control
    // This would need to be implemented via PWM on display power/backlight
    // or through software dithering
    (void)brightness;  // Unused for now
}

size_t GeekWatchDisplay::getFramebufferSize() {
    // Calculate framebuffer size in bytes
    uint16_t bytesPerRow = (DISPLAY_WIDTH + 7) / 8;
    return bytesPerRow * DISPLAY_HEIGHT;
}

void GeekWatchDisplay::sendCommand(uint8_t cmd, uint8_t *data, size_t len) {
    if (!_spi) {
        return;
    }

    digitalWrite(_cs, HIGH);
    delayMicroseconds(3);
    
    _spi->transfer(cmd);
    
    if (data && len > 0) {
        for (size_t i = 0; i < len; i++) {
            _spi->transfer(data[i]);
        }
    }
    
    _spi->transfer(0x00);  // Trailing dummy byte
    
    delayMicroseconds(3);
    digitalWrite(_cs, LOW);
}
