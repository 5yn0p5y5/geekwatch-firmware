# Pin Verification Checklist

## Your Hardware Connections (from silkscreen)
```
Display LS011B7DH03:
- SCS  = P0.09  →  Arduino pin 9
- SCLK = P1.06  →  Arduino pin 38 (32 + 6)
- SI   = P1.04  →  Arduino pin 36 (32 + 4)
- VCC  = 3.3V
- GND  = GND
```

## Current Code Configuration
```cpp
#define DISPLAY_CS_PIN      9   // P0.09
#define DISPLAY_SCK_PIN     38  // P1.06
#define DISPLAY_MOSI_PIN    36  // P1.04
```

## SPI Configuration
```cpp
SPI.setPins(0xFF, DISPLAY_SCK_PIN, DISPLAY_MOSI_PIN);
// MISO = 0xFF (not used)
// SCK  = 38 (P1.06)
// MOSI = 36 (P1.04)
```

## Sharp Memory Display Protocol
- **Mode**: SPI Mode 0 (CPOL=0, CPHA=0)
- **Bit Order**: LSB first
- **Speed**: Up to 2MHz
- **Commands**:
  - 0x20 = Clear all (M2=1)
  - 0x80 = Write data (M0=1)
  - 0x00 = VCOM toggle (M0=0)

## Troubleshooting

### Check Power
1. Verify display has 3.3V power
2. Check GND connection

### Check SPI Signals with Logic Analyzer/Scope
- CS should go HIGH during transmission
- SCK should clock at ~2MHz
- MOSI should show data

### Verify Display Works
Try connecting display to known-good hardware first to rule out hardware failure
