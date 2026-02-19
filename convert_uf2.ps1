# Convert firmware.hex to firmware.uf2 for UF2 bootloader

Write-Host "Converting firmware to UF2 format..."
python uf2conv.py .pio/build/nrf52840_supermini/firmware.hex -c -f 0xADA52840 -o firmware.uf2

if ($LASTEXITCODE -eq 0) {
    Write-Host "Success! firmware.uf2 created"
    Write-Host ""
    Write-Host "Next steps:"
    Write-Host "1. Double-tap reset button on board"
    Write-Host "2. Copy firmware.uf2 to the mounted USB drive"
    Write-Host "   Example: copy firmware.uf2 D:\"
    Write-Host "   (Replace D: with the actual drive letter shown in File Explorer)"
} else {
    Write-Host "Conversion failed"
    exit 1
}
