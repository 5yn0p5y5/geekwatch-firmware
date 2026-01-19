/**
 * @file display_simple.cpp
 * @brief Simplified display driver implementation
 */

#include "display_simple.h"

#define SHARP_CMD_WRITE_LINE    0x80
#define SHARP_CMD_VCOM          0x40
#define SHARP_CMD_CLEAR         0x20

GeekWatchDisplaySimple::GeekWatchDisplaySimple(uint8_t clk, uint8_t mosi, uint8_t cs)
    : _clk(clk),
      _mosi(mosi),
      _cs(cs),
      _vcom(false),
      _spi(nullptr),
      _framebuffer(nullptr) {
    // Constructor does minimal work
}

GeekWatchDisplaySimple::~GeekWatchDisplaySimple() {
    if (_framebuffer) {
        free(_framebuffer);
    }
}

bool GeekWatchDisplaySimple::begin() {
    // Allocate framebuffer
    size_t fbSize = getFramebufferSize();
    _framebuffer = (uint8_t*)malloc(fbSize);
    if (!_framebuffer) {
        return false;
    }
    memset(_framebuffer, 0, fbSize);

    // Configure CS pin
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, LOW);

    // Initialize SPI
    _spi = &SPI;
    _spi->begin();
    _spi->beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

    delay(10);
    clearDisplay();
    display();

    return true;
}

void GeekWatchDisplaySimple::clearDisplay() {
    if (_framebuffer) {
        memset(_framebuffer, 0, getFramebufferSize());
    }
}

void GeekWatchDisplaySimple::display() {
    if (!_framebuffer || !_spi) {
        return;
    }

    digitalWrite(_cs, HIGH);
    delayMicroseconds(3);

    uint8_t cmd = SHARP_CMD_WRITE_LINE;
    if (_vcom) {
        cmd |= SHARP_CMD_VCOM;
    }
    _spi->transfer(cmd);

    uint16_t bytesPerRow = (DISPLAY_WIDTH + 7) / 8;
    
    for (uint16_t line = 0; line < DISPLAY_HEIGHT; line++) {
        _spi->transfer(line + 1);
        
        for (uint16_t b = 0; b < bytesPerRow; b++) {
            _spi->transfer(_framebuffer[line * bytesPerRow + b]);
        }
        
        _spi->transfer(0x00);
    }

    _spi->transfer(0x00);

    delayMicroseconds(3);
    digitalWrite(_cs, LOW);
}

void GeekWatchDisplaySimple::toggleVCOM() {
    if (!_spi) {
        return;
    }

    _vcom = !_vcom;

    digitalWrite(_cs, HIGH);
    delayMicroseconds(3);
    
    uint8_t cmd = _vcom ? SHARP_CMD_VCOM : 0x00;
    _spi->transfer(cmd);
    _spi->transfer(0x00);
    
    delayMicroseconds(3);
    digitalWrite(_cs, LOW);
}

void GeekWatchDisplaySimple::setPixel(int16_t x, int16_t y, bool color) {
    if (!_framebuffer || x < 0 || y < 0 || x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }

    uint16_t bytesPerRow = (DISPLAY_WIDTH + 7) / 8;
    uint16_t byteIndex = y * bytesPerRow + (x / 8);
    uint8_t bitMask = 1 << (7 - (x % 8));

    if (color) {
        _framebuffer[byteIndex] |= bitMask;
    } else {
        _framebuffer[byteIndex] &= ~bitMask;
    }
}

void GeekWatchDisplaySimple::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, bool color) {
    for (int16_t i = x; i < x + w; i++) {
        for (int16_t j = y; j < y + h; j++) {
            setPixel(i, j, color);
        }
    }
}

size_t GeekWatchDisplaySimple::getFramebufferSize() {
    uint16_t bytesPerRow = (DISPLAY_WIDTH + 7) / 8;
    return bytesPerRow * DISPLAY_HEIGHT;
}
