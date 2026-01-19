#!/usr/bin/env python3
import serial
import time

try:
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
    print("Serial port opened. Waiting for data...")
    print("Press RESET button on the board now!\n")
    
    start_time = time.time()
    while time.time() - start_time < 15:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8', errors='ignore').strip()
            if data:
                print(f"RECEIVED: {data}")
        time.sleep(0.1)
    
    print("\nTest complete. If nothing received, there may be an issue with Serial in the firmware.")
    ser.close()
    
except Exception as e:
    print(f"Error: {e}")
    print("\nTry: sudo apt install python3-serial")
