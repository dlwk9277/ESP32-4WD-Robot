#!/usr/bin/env python3
"""
ESP32 4WD Robot - PC Control Script
Connects to robot via Bluetooth Serial and allows keyboard control

Requirements:
    pip install pyserial

Usage:
    python robot_control.py

Controls:
    W/A/S/D - Movement
    Q/E - Spin left/right
    X - Stop
    +/- - Speed control
    T - Run test sequence
    ESC - Exit
"""

import serial
import serial.tools.list_ports
import sys
import time
import os

# Platform-specific keyboard input
if os.name == 'nt':  # Windows
    import msvcrt
    def get_key():
        if msvcrt.kbhit():
            return msvcrt.getch().decode('utf-8').upper()
        return None
else:  # Linux/Mac
    import tty
    import termios
    def get_key():
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch.upper() if ch else None

class RobotController:
    def __init__(self):
        self.ser = None
        self.running = False
        
    def find_bluetooth_port(self):
        """Find the Bluetooth serial port for ESP32"""
        print("Searching for ESP32 Bluetooth device...")
        ports = serial.tools.list_ports.comports()
        
        for port in ports:
            print(f"  Found: {port.device} - {port.description}")
            # Look for ESP32 or Bluetooth-related ports
            if 'bluetooth' in port.description.lower() or 'esp32' in port.description.lower():
                return port.device
        
        # If no automatic match, show all ports
        if ports:
            print("\nAvailable ports:")
            for i, port in enumerate(ports):
                print(f"  [{i}] {port.device} - {port.description}")
            
            choice = input("\nEnter port number (or full port name like COM3): ")
            try:
                idx = int(choice)
                return ports[idx].device
            except (ValueError, IndexError):
                return choice
        
        return None
    
    def connect(self, port=None):
        """Connect to the robot"""
        if port is None:
            port = self.find_bluetooth_port()
        
        if not port:
            print("Error: No port specified")
            return False
        
        try:
            print(f"\nConnecting to {port}...")
            self.ser = serial.Serial(port, 115200, timeout=1)
            time.sleep(2)  # Wait for connection to establish
            print("✓ Connected to ESP32 4WD Robot!")
            return True
        except serial.SerialException as e:
            print(f"Error connecting: {e}")
            return False
    
    def send_command(self, cmd):
        """Send a command to the robot"""
        if self.ser and self.ser.is_open:
            self.ser.write(cmd.encode())
            print(f"Sent: {cmd}")
            
            # Read response if available
            time.sleep(0.05)
            if self.ser.in_waiting:
                response = self.ser.readline().decode('utf-8', errors='ignore').strip()
                if response:
                    print(f"Robot: {response}")
        else:
            print("Error: Not connected to robot")
    
    def print_controls(self):
        """Print control instructions"""
        print("\n" + "="*50)
        print("ESP32 4WD ROBOT CONTROL")
        print("="*50)
        print("\nMovement Controls:")
        print("  W - Forward")
        print("  S - Backward")
        print("  A - Turn Left")
        print("  D - Turn Right")
        print("  Q - Spin Left (counter-clockwise)")
        print("  E - Spin Right (clockwise)")
        print("  X - STOP")
        print("\nSpeed Controls:")
        print("  + - Increase Speed")
        print("  - - Decrease Speed")
        print("\nOther:")
        print("  T - Run Full Test Sequence")
        print("  ESC - Exit")
        print("="*50)
        print("\nReady! Press keys to control the robot...")
        print("(Commands are sent immediately, no Enter needed)\n")
    
    def run(self):
        """Main control loop"""
        self.running = True
        self.print_controls()
        
        last_key = None
        
        try:
            while self.running:
                key = get_key()
                
                if key and key != last_key:
                    last_key = key
                    
                    # Handle ESC (might be multiple chars)
                    if ord(key) == 27:  # ESC
                        print("\nExiting...")
                        self.send_command('X')  # Stop before exit
                        self.running = False
                        break
                    
                    # Valid commands
                    valid_cmds = ['W', 'A', 'S', 'D', 'Q', 'E', 'X', '+', '-', 'T']
                    if key in valid_cmds:
                        self.send_command(key)
                    elif key == 'H':  # Help
                        self.print_controls()
                
                time.sleep(0.01)
                
        except KeyboardInterrupt:
            print("\n\nInterrupted! Stopping robot...")
            self.send_command('X')
        
        finally:
            if self.ser and self.ser.is_open:
                self.ser.close()
                print("Disconnected from robot")
    
    def disconnect(self):
        """Disconnect from the robot"""
        if self.ser and self.ser.is_open:
            self.send_command('X')  # Stop motors
            self.ser.close()
            print("Disconnected")

def main():
    print("="*50)
    print("ESP32 4WD Robot - PC Controller")
    print("="*50)
    
    controller = RobotController()
    
    # Try to connect
    if controller.connect():
        # Run the control loop
        controller.run()
    else:
        print("\nFailed to connect to robot.")
        print("\nTroubleshooting:")
        print("1. Make sure the robot is powered on")
        print("2. Pair with 'ESP32_4WD_Robot' in Bluetooth settings first")
        print("3. Check that the correct COM port is selected")
        print("4. On Windows, you may need to check Device Manager")
        print("5. On Linux, you may need: sudo usermod -a -G dialout $USER")
    
    print("\nGoodbye!")

if __name__ == "__main__":
    main()
