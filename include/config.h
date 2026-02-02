/**
 * @file config.h
 * @brief Hardware configuration for GeekWatch v1
 * 
 * Hardware:
 * - NRF52840 ProMicro board
 * - MAX98357A I2S audio amplifier + speaker
 * - LS011B7DH03 160x68 Sharp Memory Display (niceview clone)
 */

#ifndef CONFIG_H
#define CONFIG_H

// ========== Display Configuration (SPI) ==========
// LS011B7DH03 Sharp Memory Display - 160x68 pixels
#define DISPLAY_WIDTH       160
#define DISPLAY_HEIGHT      68

// SPI pins for display
// Silkscreen: SCS=P0.09, SCLK=P1.06, SI=P1.04
#define DISPLAY_CS_PIN      9   // P0.09 - Chip select (SCS)
#define DISPLAY_SCK_PIN     38  // P1.06 - SPI clock (SCLK)
#define DISPLAY_MOSI_PIN    36  // P1.04 - SPI MOSI (SI)
// Note: Sharp memory displays don't use MISO

// Some nice!view clones have an enable pin that must be HIGH
// If your display has a DISP or EN pin, connect it to a GPIO and define it here
// #define DISPLAY_EN_PIN   10  // Example: P0.10 - Enable pin (set HIGH)

// Display refresh rate (Hz) - Sharp displays need periodic VCOM toggle
#define DISPLAY_REFRESH_RATE 1

// ========== Audio Configuration (I2S) ==========
// MAX98357A I2S Audio Amplifier
// Silkscreen: LRC=P0.11, BCLK=P1.00, DIN=P0.24, GAIN=P0.22, SD=P0.20
#define I2S_LRCK_PIN        11  // P0.11 - I2S word select (LRC)
#define I2S_SCK_PIN         32  // P1.00 - I2S bit clock (BCLK)
#define I2S_DIN_PIN         24  // P0.24 - I2S data (DIN)
#define I2S_GAIN_PIN        22  // P0.22 - Gain control (optional)
#define I2S_SD_PIN          20  // P0.20 - Shutdown pin (active high)

// Audio sample rate and bit depth
#define AUDIO_SAMPLE_RATE   16000  // 16kHz for speech
#define AUDIO_BIT_DEPTH     16     // 16-bit samples

// ========== Power Management ==========
#define ENABLE_LOW_POWER_MODE  true
#define SLEEP_TIMEOUT_MS       30000  // 30 seconds
#define DISPLAY_UPDATE_MS      1000   // Update display every 1 second when running
#define DISPLAY_IDLE_MS        5000   // Reduce updates when idle (stopwatches paused)

// ========== Debug Configuration ==========
// Set to false for production to save significant power
#define DEBUG_SERIAL        false
#define SERIAL_BAUD_RATE    115200

// ========== Firmware Version ==========
#define FIRMWARE_VERSION    "1.0.0"
#define HARDWARE_VERSION    "v1"

#endif // CONFIG_H
