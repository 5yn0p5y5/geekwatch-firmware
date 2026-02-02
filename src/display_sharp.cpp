/**
 * @file display_sharp.cpp
 * @brief Implementation of Sharp Memory Display driver
 */

#include "display_sharp.h"

SharpDisplay::SharpDisplay() : vcomState(false) {
    memset(framebuffer, 0, sizeof(framebuffer));
}

bool SharpDisplay::begin() {
    #if DEBUG_SERIAL
    Serial.println("Display: Initializing 3-wire SPI Sharp Memory Display...");
    #endif
    
    // CS starts LOW, goes HIGH for entire command frame
    pinMode(DISPLAY_CS_PIN, OUTPUT);
    digitalWrite(DISPLAY_CS_PIN, LOW);
    #if DEBUG_SERIAL
    Serial.print("  CS pin (");
    Serial.print(DISPLAY_CS_PIN);
    Serial.println("): LOW (idle)");
    
    // Configure custom SPI pins
    Serial.print("  SPI pins - SCK: ");
    Serial.print(DISPLAY_SCK_PIN);
    Serial.print(", SI: ");
    Serial.println(DISPLAY_MOSI_PIN);
    #endif
    
    SPI.setPins(0xFF, DISPLAY_SCK_PIN, DISPLAY_MOSI_PIN);
    SPI.begin();
    
    // Sharp Memory Display: LSB first per datasheet, Mode 0, max 2MHz
    // Use 500kHz for lower power consumption
    SPI.beginTransaction(SPISettings(500000, LSBFIRST, SPI_MODE0));
    #if DEBUG_SERIAL
    Serial.println("  SPI: 500kHz, LSB-first, Mode 0 (3-wire protocol)");
    #endif
    
    delay(100);
    
    // Clear screen
    #if DEBUG_SERIAL
    Serial.println("  Clearing display...");
    #endif
    clearDisplay();
    
    // End transaction to save power when not in use
    SPI.endTransaction();
    
    #if DEBUG_SERIAL
    Serial.println("Display: Ready!");
    #endif
    
    return true;
}

void SharpDisplay::clearDisplay() {
    #if DEBUG_SERIAL
    Serial.println("Clear (all black)");
    #endif
    
    // Start SPI transaction
    SPI.beginTransaction(SPISettings(500000, LSBFIRST, SPI_MODE0));
    
    // 3-wire SPI: CS HIGH for entire frame
    delayMicroseconds(3);
    digitalWrite(DISPLAY_CS_PIN, HIGH);
    delayMicroseconds(3);
    
    // Command: M2=1 (clear 0x04), M1=VCOM (0x02) - LSB-first format
    uint8_t cmd = 0x04 | (vcomState ? 0x02 : 0x00);
    SPI.transfer(cmd);
    SPI.transfer(0x00);  // Trailer
    
    delayMicroseconds(1);
    digitalWrite(DISPLAY_CS_PIN, LOW);
    
    // End SPI transaction to save power
    SPI.endTransaction();
    
    vcomState = !vcomState;
    memset(framebuffer, 0, sizeof(framebuffer));
}

void SharpDisplay::setPixel(uint8_t x, uint8_t y, bool white) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    
    uint8_t byteIndex = x / 8;
    uint8_t bitIndex = x % 8;
    
    // 0xFF = white, 0x00 = black
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
    // Start SPI transaction only when needed
    SPI.beginTransaction(SPISettings(500000, LSBFIRST, SPI_MODE0));
    
    digitalWrite(DISPLAY_CS_PIN, HIGH);
    delayMicroseconds(10);
    
    // Send write command (LSB-first: 0x01 = write, 0x02 = VCOM)
    uint8_t cmd = 0x01 | (vcomState ? 0x02 : 0x00);
    SPI.transfer(cmd);
    
    // Send all lines (1-based addressing)
    for (uint8_t y = 0; y < DISPLAY_HEIGHT; y++) {
        // Line address (1-based) - no reversal needed with LSBFIRST
        SPI.transfer(y + 1);
        
        // Line data - no reversal needed with LSBFIRST
        for (int i = 0; i < DISPLAY_WIDTH / 8; i++) {
            SPI.transfer(framebuffer[y][i]);
        }
        
        // Dummy byte after each line
        SPI.transfer(0x00);
    }
    
    // Final trailer byte
    SPI.transfer(0x00);
    
    delayMicroseconds(10);
    digitalWrite(DISPLAY_CS_PIN, LOW);
    
    // End SPI transaction to save power
    SPI.endTransaction();
    
    vcomState = !vcomState;
}

void SharpDisplay::toggleVCOM() {
    // VCOM toggle only (no data write)
    SPI.beginTransaction(SPISettings(500000, LSBFIRST, SPI_MODE0));
    
    delayMicroseconds(3);
    digitalWrite(DISPLAY_CS_PIN, HIGH);
    delayMicroseconds(3);
    
    // Command: M1=VCOM (0x02), M0=0, M2=0 - LSB-first format
    uint8_t cmd = vcomState ? 0x02 : 0x00;
    SPI.transfer(cmd);
    SPI.transfer(0x00);
    
    delayMicroseconds(1);
    digitalWrite(DISPLAY_CS_PIN, LOW);
    
    SPI.endTransaction();
    
    vcomState = !vcomState;
}

void SharpDisplay::fillScreen(bool white) {
    #if DEBUG_SERIAL
    Serial.print("Filling ");
    Serial.print(white ? "WHITE" : "BLACK");
    Serial.print(" - ");
    Serial.print(DISPLAY_HEIGHT);
    Serial.println(" lines");
    #endif
    
    SPI.beginTransaction(SPISettings(500000, LSBFIRST, SPI_MODE0));
    
    // 3-wire SPI: CS HIGH for entire frame
    delayMicroseconds(3);  // tsSCS min = 3us
    digitalWrite(DISPLAY_CS_PIN, HIGH);
    delayMicroseconds(3);  // CS setup before data
    
    // Command byte (LSB-first): M0=write (0x01), M1=VCOM (0x02)
    uint8_t cmd = 0x01 | (vcomState ? 0x02 : 0x00);
    #if DEBUG_SERIAL
    Serial.print("  CMD byte: 0b");
    Serial.print(cmd, BIN);
    Serial.print(" (0x");
    Serial.print(cmd, HEX);
    Serial.print(") VCOM=");
    Serial.print(vcomState ? "1" : "0");
    Serial.println(")");
    #endif
    
    SPI.transfer(cmd);
    
    // Correct polarity: 0xFF = white (pixel on), 0x00 = black (pixel off)
    uint8_t pixelByte = white ? 0xFF : 0x00;
    #if DEBUG_SERIAL
    Serial.print("  Pixel byte: 0x");
    Serial.println(pixelByte, HEX);
    Serial.print("  Sending ");
    Serial.print(DISPLAY_HEIGHT);
    Serial.println(" lines...");
    #endif
    
    // Send all lines (1-based addressing)
    for (uint8_t line = 1; line <= DISPLAY_HEIGHT; line++) {
        SPI.transfer(line);  // Line address (1-based) - no reversal with LSBFIRST
        
        // 160 pixels = 20 bytes per line - no reversal with LSBFIRST
        for (int i = 0; i < 20; i++) {
            SPI.transfer(pixelByte);
        }
        
        // Dummy byte after each line
        SPI.transfer(0x00);
    }
    
    // Trailer (8 dummy bits minimum)
    SPI.transfer(0x00);
    
    #if DEBUG_SERIAL
    Serial.println("  Frame complete, CS going LOW");
    #endif
    delayMicroseconds(1);  // thSCS min = 1us  
    digitalWrite(DISPLAY_CS_PIN, LOW);  // CS low completes frame
    
    SPI.endTransaction();
    
    vcomState = !vcomState;
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
