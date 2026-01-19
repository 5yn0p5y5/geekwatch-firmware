# geekwatch Firmware

geekwatch is a device and a service that help you stay productive!

## Hardware (v1)

- **MCU**: nRF52840 ProMicro board (with Adafruit UF2 bootloader)
- **Display**: LS011B7DH03 160x68 Sharp Memory Display (SPI)
- **Audio**: MAX98357A I2S audio amplifier + speaker
- **Power**: USB-C charging, LiPo battery (TBD)

## Development Setup

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- USB cable for programming via UF2 bootloader

### Building the Firmware

```bash
# Build the project
pio run

# Build and upload via UF2 bootloader
# 1. Double-tap reset button on nRF52840 ProMicro to enter bootloader mode
# 2. Board will appear as USB drive (e.g., "PROMICROBOOT")
# 3. Run upload command:
pio run --target upload

# Monitor serial output
pio device monitor
```

### Project Structure

```
geekwatch-firmware/
├── boards/
│   └── nrf52840_promicro.json    # Custom board definition
├── include/
│   ├── config.h                   # Hardware configuration
│   ├── display.h                  # Display driver header
│   └── audio.h                    # Audio driver header
├── src/
│   ├── main.cpp                   # Main application
│   ├── display.cpp                # Display driver implementation
│   └── audio.cpp                  # Audio driver implementation
├── platformio.ini                 # PlatformIO configuration
└── README.md
```

## Pin Configuration

Verify these pin assignments match your hardware wiring:

### Display (SPI)
- SCK: Pin 13
- MOSI: Pin 12
- CS: Pin 11

### Audio (I2S)
- SCK (BCLK): Pin 26
- LRCK (WS): Pin 25
- DIN (SD): Pin 27

Edit [include/config.h](include/config.h) to modify pin assignments.

## Current Status

### ✅ Implemented
- PlatformIO project structure
- Custom nRF52840 ProMicro board definition
- Display driver scaffold (SPI communication)
- Audio driver scaffold (I2S interface)
- Basic firmware application with:
  - Display initialization
  - Splash screen
  - Test pattern
  - Time display (uptime counter)
  - VCOM toggle for Sharp display

### ⚠️ Partial/Needs Work
- **I2S Audio**: Hardware configuration requires Nordic SDK register access or native I2S library
  - Framework is in place, but actual I2S peripheral setup needs implementation
  - Options: Direct register access, Nordic SDK integration, or Arduino I2S library
- **Display Driver**: Based on Sharp Memory Display protocol, may need tuning for LS011B7DH03 specifics

### 🔲 To Do
- Complete I2S hardware initialization
- Test audio playback
- Implement button handling
- Add menu system
- Battery monitoring
- BLE connectivity
- Low power/sleep modes
- Real-time clock integration
- Notification system

## Flashing Instructions

### Via UF2 Bootloader (Recommended)

1. **Enter bootloader mode**:
   - Double-tap the reset button on the nRF52840 ProMicro
   - Board LED should pulse, indicating bootloader mode
   - A USB drive named "PROMICROBOOT" (or similar) will appear

2. **Build firmware**:
   ```bash
   pio run
   ```

3. **Flash to device**:
   ```bash
   pio run --target upload
   ```
   
   Or manually:
   - Locate the compiled UF2 file in `.pio/build/geekwatch/firmware.uf2`
   - Drag and drop onto the USB drive
   - Board will automatically reset and run new firmware

### Serial Monitor

```bash
pio device monitor
```

Default baud rate: 115200

## Notes

### I2S Implementation

The current audio driver is a scaffold. For full I2S functionality, you have three options:

1. **Direct Register Access** (Most control):
   - Use nRF52840 registers directly (`NRF_I2S->PSEL.SCK`, etc.)
   - Requires understanding Nordic register map
   - Best performance and flexibility

2. **Nordic SDK Integration** (Recommended for production):
   - Integrate Nordic nRF5 SDK I2S driver
   - Well-tested, full-featured
   - Adds complexity to build system

3. **Arduino I2S Library** (Easiest):
   - Look for `Adafruit_ZeroI2S` or similar
   - May need porting from SAMD to nRF52
   - Limited functionality but simpler

### Display Protocol

The LS011B7DH03 uses the Sharp Memory Display protocol. The current driver implementation:
- Assumes standard Sharp protocol
- May need adjustments based on specific niceview clone behavior
- Test with actual hardware and adjust timing if needed

## License

See [LICENSE](LICENSE) file for details.

## Version History

- **v1.0.0** (2026-01-18): Initial scaffold implementation
  - Basic display and audio drivers
  - PlatformIO project setup
  - UF2 bootloader workflow