#!/bin/bash
# Convert firmware.hex to firmware.uf2 for UF2 bootloader

echo "Converting firmware to UF2 format..."
python3 uf2conv.py .pio/build/geekwatch/firmware.hex -c -f 0xADA52840 -o firmware.uf2

if [ $? -eq 0 ]; then
    echo "✓ Success! firmware.uf2 created"
    echo ""
    echo "Next steps:"
    echo "1. Double-tap reset button on board"
    echo "2. Copy firmware.uf2 to the mounted USB drive"
    echo "   Example: cp firmware.uf2 /media/$USER/NRF52BOOT/"
else
    echo "✗ Conversion failed"
    exit 1
fi
