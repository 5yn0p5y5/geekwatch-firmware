/**
 * @file display_sharp.cpp
 * @brief Implementation of Sharp Memory Display driver
 */

#include "display_sharp.h"

SharpDisplay::SharpDisplay() : vcomState(false) {
    memset(framebuffer, 0, sizeof(framebuffer));
}

bool SharpDisplay::begin() {
    // Initialize CS pin
    pinMode(DISPLAY_CS_PIN, OUTPUT);
    digitalWrite(DISPLAY_CS_PIN, LOW);
    
    // Initialize SPI
    SPI.begin();
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    
    delay(100);
    
    // Clear the display
    clearDisplay();
    
    return true;
}

void SharpDisplay::clearDisplay() {
    digitalWrite(DISPLAY_CS_PIN, HIGH);
    delayMicroseconds(10);
    
    // Send clear command (0x04 | VCOM)
    uint8_t cmd = 0x04 | (vcomState ? 0x40 : 0x00);
    SPI.transfer(cmd);
    SPI.transfer(0x00); // Trailer byte
    
    delayMicroseconds(10);
    digitalWrite(DISPLAY_CS_PIN, LOW);
    
    // Clear framebuffer
    memset(framebuffer, 0, sizeof(framebuffer));
    
    vcomState = !vcomState;
}

void SharpDisplay::setPixel(uint8_t x, uint8_t y, bool white) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    
    uint8_t byteIndex = x / 8;
    uint8_t bitIndex = x % 8;
    
    if (white) {
        framebuffer[y][byteIndex] |= (1 << bitIndex);
    } else {
        framebuffer[y][byteIndex] &= ~(1 << bitIndex);
    }
}

void SharpDisplay::drawLine(uint8_t y, const uint8_t* lineData) {
    if (y >= DISPLAY_HEIGHT) return;
    
    digitalWrite(DISPLAY_CS_PIN, HIGH);
    delayMicroseconds(10);
    
    // Send write command
    uint8_t cmd = 0x80 | (vcomState ? 0x40 : 0x00);
    SPI.transfer(cmd);
    
    // Send line address (1-based)
    SPI.transfer(reverseByte(y + 1));
    
    // Send line data (20 bytes for 160 pixels)
    for (int i = 0; i < DISPLAY_WIDTH / 8; i++) {
        SPI.transfer(reverseByte(lineData[i]));
    }
    
    // Send trailer
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    
    delayMicroseconds(10);
    digitalWrite(DISPLAY_CS_PIN, LOW);
}

void SharpDisplay::refresh() {
    digitalWrite(DISPLAY_CS_PIN, HIGH);
    delayMicroseconds(10);
    
    // Send write command
    uint8_t cmd = 0x80 | (vcomState ? 0x40 : 0x00);
    SPI.transfer(cmd);
    
    // Send all lines
    for (uint8_t y = 0; y < DISPLAY_HEIGHT; y++) {
        // Line address (1-based)
        SPI.transfer(reverseByte(y + 1));
        
        // Line data
        for (int i = 0; i < DISPLAY_WIDTH / 8; i++) {
            SPI.transfer(reverseByte(framebuffer[y][i]));
        }
    }
    
    // Trailer bytes
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    
    delayMicroseconds(10);
    digitalWrite(DISPLAY_CS_PIN, LOW);
    
    vcomState = !vcomState;
}

void SharpDisplay::toggleVCOM() {
    digitalWrite(DISPLAY_CS_PIN, HIGH);
    delayMicroseconds(10);
    
    // Send VCOM toggle command
    uint8_t cmd = 0x00 | (vcomState ? 0x40 : 0x00);
    SPI.transfer(cmd);
    SPI.transfer(0x00);
    
    delayMicroseconds(10);
    digitalWrite(DISPLAY_CS_PIN, LOW);
    
    vcomState = !vcomState;
}

void SharpDisplay::fillScreen(bool white) {
    memset(framebuffer, white ? 0xFF : 0x00, sizeof(framebuffer));
    refresh();
}

void SharpDisplay::drawTestPattern() {
    // Draw horizontal lines
    for (int y = 0; y < DISPLAY_HEIGHT; y += 4) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            setPixel(x, y, true);
        }
    }
    
    // Draw vertical lines
    for (int x = 0; x < DISPLAY_WIDTH; x += 8) {
        for (int y = 0; y < DISPLAY_HEIGHT; y++) {
            setPixel(x, y, true);
        }
    }
    
    // Draw border
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
        setPixel(x, 0, true);
        setPixel(x, DISPLAY_HEIGHT - 1, true);
    }
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        setPixel(0, y, true);
        setPixel(DISPLAY_WIDTH - 1, y, true);
    }
    
    refresh();
}

// Sharp displays need LSB first bit ordering
uint8_t SharpDisplay::reverseByte(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}
