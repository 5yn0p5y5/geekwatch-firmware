/**
 * @file audio.h
 * @brief I2S audio driver for MAX98357A amplifier
 * 
 * The MAX98357A is a simple I2S DAC/amplifier that doesn't require I2C configuration.
 * Just provide I2S data stream (BCLK, LRCLK, DIN) and it outputs amplified audio.
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "config.h"

// Audio buffer configuration
#define AUDIO_BUFFER_SIZE   512   // Samples per buffer
#define NUM_AUDIO_BUFFERS   2     // Double buffering

class GeekWatchAudio {
public:
    /**
     * @brief Constructor
     * @param sck I2S bit clock pin (BCLK)
     * @param lrck I2S word select pin (LRCLK/WS)
     * @param din I2S data pin (DIN/SD)
     */
    GeekWatchAudio(uint8_t sck, uint8_t lrck, uint8_t din);

    /**
     * @brief Initialize I2S audio subsystem
     * @param sampleRate Sample rate in Hz (e.g., 16000, 44100)
     * @param bitDepth Bit depth (16 or 32)
     * @return true if successful, false otherwise
     */
    bool begin(uint32_t sampleRate = AUDIO_SAMPLE_RATE, 
               uint8_t bitDepth = AUDIO_BIT_DEPTH);

    /**
     * @brief Stop I2S audio
     */
    void end();

    /**
     * @brief Play audio samples
     * @param samples Pointer to audio sample buffer
     * @param numSamples Number of samples to play
     * @param blocking If true, wait for playback to complete
     * @return true if samples were queued/played
     */
    bool play(const int16_t *samples, size_t numSamples, bool blocking = false);

    /**
     * @brief Play a tone
     * @param frequency Frequency in Hz
     * @param duration Duration in milliseconds
     */
    void playTone(uint16_t frequency, uint32_t duration);

    /**
     * @brief Check if audio is currently playing
     * @return true if audio is playing
     */
    bool isPlaying();

    /**
     * @brief Set volume (0-255)
     * @param volume Volume level
     * @note MAX98357A has fixed gain, so this may need software scaling
     */
    void setVolume(uint8_t volume);

    /**
     * @brief Stop any currently playing audio
     */
    void stop();

private:
    uint8_t _sck, _lrck, _din;
    uint32_t _sampleRate;
    uint8_t _bitDepth;
    uint8_t _volume;
    bool _isInitialized;
    bool _isPlaying;

    // Audio buffers
    int16_t *_audioBuffer[NUM_AUDIO_BUFFERS];
    volatile uint8_t _currentBuffer;

    /**
     * @brief Configure nRF52 I2S peripheral
     */
    bool configureI2S();

    /**
     * @brief Generate tone samples
     * @param samples Output buffer
     * @param numSamples Number of samples to generate
     * @param frequency Frequency in Hz
     */
    void generateTone(int16_t *samples, size_t numSamples, uint16_t frequency);

    /**
     * @brief Apply volume scaling to samples
     * @param samples Sample buffer
     * @param numSamples Number of samples
     */
    void applyVolume(int16_t *samples, size_t numSamples);
};

#endif // AUDIO_H
