# ESP32 4-Wheel Drive Robot with TFT Display

A Bluetooth-controlled 4WD robot using ESP32, dual L298N motor drivers, and a 2.4" TFT display for real-time status monitoring.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-green.svg)

## Features

- 🤖 4-wheel independent motor control via 2x L298N drivers
- 📱 Bluetooth control from PC/smartphone
- 🖥️ 2.4" TFT display showing real-time status
- ⚡ 6V battery powered
- 🎮 Simple command protocol for movement

## Hardware Requirements

### Components
- **ESP32 with Expansion Board** (with voltage selection jumper)
- **2x L298N Motor Driver Modules**
- **4x DC Motors** (6V compatible)
- **2.4" TFT SPI Display** (240x320, ILI9341 chipset)
- **6V Battery Pack**
- **Breadboard(s)** for power distribution
- **Jumper wires**

### Tools Needed
- Soldering iron (optional, for permanent connections)
- Wire strippers
- Screwdriver for L298N terminals

## Wiring Diagram

### Power Distribution

```
[6V Battery Pack]
     |
     +---> (+) Breadboard Rail ---> L298N #1 (12V terminal)
     |                         ---> L298N #2 (12V terminal)  
     |                         ---> ESP32 "V" terminal
     |
     +---> (-) Breadboard Rail ---> L298N #1 GND
                               ---> L298N #2 GND
                               ---> ESP32 GND
```

**ESP32 Expansion Board Jumper Setting:**
- Set jumper to connect: **5V ↔ V**
- Leave 3.3V disconnected

### Motor Connections

**L298N #1 (Left Motors):**
- OUT1 & OUT2: Front-Left Motor
- OUT3 & OUT4: Rear-Left Motor

**L298N #2 (Right Motors):**
- OUT1 & OUT2: Front-Right Motor
- OUT3 & OUT4: Rear-Right Motor

### ESP32 to L298N Control Pins

**L298N #1 (Left Side):**
| ESP32 Pin | L298N #1 Pin | Function |
|-----------|--------------|----------|
| GPIO 25 | ENA | Left motors speed (PWM) |
| GPIO 26 | IN1 | Front-left direction A |
| GPIO 27 | IN2 | Front-left direction B |
| GPIO 14 | IN3 | Rear-left direction A |
| GPIO 12 | IN4 | Rear-left direction B |
| GPIO 13 | ENB | Left motors speed (PWM) |

**L298N #2 (Right Side):**
| ESP32 Pin | L298N #2 Pin | Function |
|-----------|--------------|----------|
| GPIO 33 | ENA | Right motors speed (PWM) |
| GPIO 32 | IN1 | Front-right direction A |
| GPIO 21 | IN2 | Front-right direction B |
| GPIO 22 | IN3 | Rear-right direction A |
| GPIO 5 | IN4 | Rear-right direction B |
| GPIO 19 | ENB | Right motors speed (PWM) |

### TFT Display Connections

| TFT Pin | ESP32 Pin | Notes |
|---------|-----------|-------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| CS | GPIO 15 | Chip Select |
| RESET | GPIO 4 | Reset |
| DC | GPIO 2 | Data/Command |
| MOSI | GPIO 23 | SPI Data Out |
| SCK | GPIO 18 | SPI Clock |
| LED | 3.3V | Backlight (always on) |
| MISO | GPIO 16 | SPI Data In (optional) |

## Software Setup

### Prerequisites

1. **Arduino IDE** (1.8.x or 2.x)
2. **ESP32 Board Support:**
   - Open Arduino IDE
   - Go to File → Preferences
   - Add to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Tools → Board → Boards Manager → Search "ESP32" → Install

3. **Required Libraries:**
   - `Adafruit GFX Library` (by Adafruit)
   - `Adafruit ILI9341` (by Adafruit)
   - `BluetoothSerial` (included with ESP32 board package)

### Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/esp32-4wd-robot.git
   cd esp32-4wd-robot
   ```

2. Open `esp32_robot_test.ino` in Arduino IDE

3. Select your board:
   - Tools → Board → ESP32 Arduino → ESP32 Dev Module

4. Select your port:
   - Tools → Port → (your ESP32 COM port)

5. Upload the code

## Usage

### Bluetooth Pairing

1. Power on the robot
2. On your PC/phone, search for Bluetooth devices
3. Look for device named: **"ESP32_4WD_Robot"**
4. Pair with it (no PIN required)

### Control Commands

Send single character commands via Bluetooth Serial:

| Command | Action |
|---------|--------|
| `W` | Forward |
| `S` | Backward |
| `A` | Turn Left |
| `D` | Turn Right |
| `Q` | Spin Left |
| `E` | Spin Right |
| `X` | Stop |
| `+` | Increase Speed |
| `-` | Decrease Speed |
| `T` | Run Full Test Sequence |

### Python Control Script

Use the included `robot_control.py` for PC control:

```bash
pip install pyserial
python robot_control.py
```

Then use keyboard controls: W/A/S/D/Q/E for movement, X to stop.

### TFT Display Information

The display shows:
- Current command received
- Motor speed (0-255)
- Connection status
- Real-time motor direction indicators

## Testing

### Initial Test Procedure

1. **Power Test:**
   - Connect 6V battery
   - Check TFT display powers on
   - Verify "Waiting..." message appears

2. **Bluetooth Test:**
   - Pair with ESP32
   - Send 'X' command
   - Display should show "STOP"

3. **Motor Test:**
   - Send 'T' command to run automated test
   - All 4 motors should spin in sequence
   - Display updates with each test step

4. **Manual Control:**
   - Send W/S/A/D commands
   - Verify motors respond correctly
   - Adjust motor polarity if spinning wrong direction

### Troubleshooting

**Motors don't spin:**
- Check all GND connections (must be common!)
- Verify L298N jumpers are installed (enable 5V regulator)
- Check motor power connections to battery
- Test motor voltage at L298N outputs

**Display blank:**
- Verify 3.3V power to TFT
- Check SPI connections (especially CS, DC, RESET)
- Try adjusting display initialization code

**Bluetooth won't pair:**
- Restart ESP32
- Check Serial Monitor for "Bluetooth Ready" message
- Some PCs require "Add Bluetooth device" in settings

**Motors spin wrong direction:**
- Swap motor wire polarity at L298N terminals
- Or modify code: swap IN1/IN2 or IN3/IN4 values

## Code Structure

```
esp32-4wd-robot/
├── README.md
├── esp32_robot_test/
│   └── esp32_robot_test.ino    # Main Arduino sketch
├── python_control/
│   └── robot_control.py         # PC control script
├── wiring/
│   └── wiring_diagram.png       # Visual wiring guide
└── LICENSE
```

## Customization

### Adjusting Speed
Edit these constants in the code:
```cpp
int motorSpeed = 200;      // Default speed (0-255)
const int MAX_SPEED = 255;
const int MIN_SPEED = 100;
```

### Changing Bluetooth Name
```cpp
SerialBT.begin("ESP32_4WD_Robot"); // Change name here
```

### Display Colors
```cpp
#define COLOR_BG ILI9341_BLACK
#define COLOR_TEXT ILI9341_WHITE
#define COLOR_FWD ILI9341_GREEN
#define COLOR_REV ILI9341_RED
```

## Safety Notes

⚠️ **Important Safety Guidelines:**
- Always test motors individually before full assembly
- Use proper gauge wire for motor currents
- Don't exceed 12V input to L298N
- Ensure all GND connections are secure
- Add a physical power switch for emergency stop
- L298N drivers can get HOT - ensure ventilation

## Future Enhancements

- [ ] Add ultrasonic sensor for obstacle avoidance
- [ ] Implement PID speed control
- [ ] Add battery voltage monitoring
- [ ] Create mobile app interface
- [ ] Add camera module
- [ ] Implement autonomous navigation

## Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Open a Pull Request

## License

This project is licensed under the MIT License - see LICENSE file for details.

## Acknowledgments

- Adafruit for excellent display libraries
- ESP32 community for Bluetooth support
- Everyone who tests and improves this project

## Contact

Questions? Open an issue or reach out!

---

**Happy Building! 🤖**
