# GeekWatch Pin Reference

## Your Hardware Connections

### Display (LS011B7DH03)
- **SCS** (Chip Select): Connected to silkscreen **P0.09**
- **SCLK** (SPI Clock): Connected to silkscreen **P1.06** 
- **SI** (SPI MOSI): Connected to silkscreen **P1.04**

### Audio (MAX98357A)
- **LRC** (Word Select): Connected to silkscreen **P0.11**
- **BCLK** (Bit Clock): Connected to silkscreen **P1.00**
- **DIN** (Data): Connected to silkscreen **P0.24**
- **GAIN**: Connected to silkscreen **P0.22**
- **SD** (Shutdown): Connected to silkscreen **P0.20**

## Pin Number Conversion

nRF52840 has ports P0 and P1:
- **P0.XX** → Arduino pin **XX** (0-31)
- **P1.XX** → Arduino pin **(32 + XX)**

## Your config.h Settings

Display:
- CS: Pin 9 (P0.09) ✓ CORRECT
- SCK: Pin 38 (P1.06) ✓ CORRECT  
- MOSI: Pin 36 (P1.04) ✓ CORRECT

Audio:
- LRCK: Pin 11 (P0.11) ✓ CORRECT
- SCK: Pin 32 (P1.00) ✓ CORRECT
- DIN: Pin 24 (P0.24) ✓ CORRECT
- GAIN: Pin 22 (P0.22) ✓ CORRECT
- SD: Pin 20 (P0.20) ✓ CORRECT

**All pin mappings are CORRECT!**

## Arduino Pin to Physical GPIO Mapping

The Adafruit nRF52 Arduino core maps pins directly:
- Arduino digitalRead(9) = nRF GPIO P0.09
- Arduino digitalRead(38) = nRF GPIO P1.06
- etc.

So your pin configuration in config.h is accurate.
