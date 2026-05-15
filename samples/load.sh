#!/bin/bash
# Auto-generated picotool script for multi-audio binary
# Binary file: buttercup.bin
# Files: 1, Sample Rate: 44100 Hz
# File size: 728,480 bytes (0.69 MB)
# Flash address: 0x10200000

echo "Loading multi-audio binary to flash..."
echo "File: buttercup.bin"
echo "Size: 728,480 bytes (0.69 MB)"
echo "Address: 0x10200000"
echo "Files: 1"
echo ""
picotool load -x buttercup.bin -o 0x10200000
echo "Multi-audio binary loaded successfully!"
