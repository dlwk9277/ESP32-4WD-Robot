/*
 * ESP32 4-Wheel Drive Robot - Complete Test System
 * 
 * Hardware:
 * - ESP32 with Expansion Board
 * - 2x L298N Motor Drivers
 * - 4x DC Motors (6V)
 * - 2.4" TFT Display (ILI9341)
 * - 6V Battery Pack
 * 
 * Control via Bluetooth Serial:
 * W=Forward, S=Back, A=Left, D=Right, Q=SpinLeft, E=SpinRight
 * X=Stop, +=Speed Up, -=Speed Down, T=Full Test
 */

#include <BluetoothSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` and enable it
#endif

// TFT Display Pins
#define TFT_CS   15
#define TFT_RST  4
#define TFT_DC   2
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_MISO 16

// L298N #1 (Left Motors) Pins
#define L1_ENA  25  // PWM Speed Control
#define L1_IN1  26  // Front-Left Motor Direction
#define L1_IN2  27
#define L1_IN3  14  // Rear-Left Motor Direction
#define L1_IN4  12
#define L1_ENB  13  // PWM Speed Control

// L298N #2 (Right Motors) Pins
#define R2_ENA  33  // PWM Speed Control
#define R2_IN1  32  // Front-Right Motor Direction
#define R2_IN2  21
#define R2_IN3  22  // Rear-Right Motor Direction
#define R2_IN4  5
#define R2_ENB  19  // PWM Speed Control

// PWM Configuration
const int PWM_FREQ = 5000;
const int PWM_RES = 8;
const int PWM_CH_L1 = 0;
const int PWM_CH_L2 = 1;
const int PWM_CH_R1 = 2;
const int PWM_CH_R2 = 3;

// Speed Control
int motorSpeed = 200;  // Default speed (0-255)
const int MAX_SPEED = 255;
const int MIN_SPEED = 100;
const int SPEED_STEP = 25;

// Display Colors
#define COLOR_BG      ILI9341_BLACK
#define COLOR_TEXT    ILI9341_WHITE
#define COLOR_FWD     ILI9341_GREEN
#define COLOR_REV     ILI9341_RED
#define COLOR_TURN    ILI9341_YELLOW
#define COLOR_STOP    ILI9341_ORANGE

// Objects
BluetoothSerial SerialBT;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Function Prototypes
void setupMotors();
void setupDisplay();
void stopMotors();
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void spinLeft();
void spinRight();
void setMotorSpeed(int speed);
void updateDisplay(String cmd, String status);
void runFullTest();
void drawMotorStatus(int x, int y, String label, bool fwd, bool rev);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 4WD Robot Starting...");
  
  // Initialize Motors
  setupMotors();
  
  // Initialize Display
  setupDisplay();
  
  // Initialize Bluetooth
  SerialBT.begin("ESP32_4WD_Robot");
  Serial.println("Bluetooth Ready! Device name: ESP32_4WD_Robot");
  
  updateDisplay("READY", "Waiting for BT...");
  delay(1000);
}

void loop() {
  // Check for Bluetooth commands
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    Serial.print("Received: ");
    Serial.println(cmd);
    
    switch(cmd) {
      case 'W':
      case 'w':
        moveForward();
        updateDisplay("FORWARD", "Moving ahead");
        break;
        
      case 'S':
      case 's':
        moveBackward();
        updateDisplay("BACKWARD", "Reversing");
        break;
        
      case 'A':
      case 'a':
        turnLeft();
        updateDisplay("TURN LEFT", "Turning...");
        break;
        
      case 'D':
      case 'd':
        turnRight();
        updateDisplay("TURN RIGHT", "Turning...");
        break;
        
      case 'Q':
      case 'q':
        spinLeft();
        updateDisplay("SPIN LEFT", "Rotating CCW");
        break;
        
      case 'E':
      case 'e':
        spinRight();
        updateDisplay("SPIN RIGHT", "Rotating CW");
        break;
        
      case 'X':
      case 'x':
        stopMotors();
        updateDisplay("STOP", "Motors stopped");
        break;
        
      case '+':
        motorSpeed = min(motorSpeed + SPEED_STEP, MAX_SPEED);
        setMotorSpeed(motorSpeed);
        updateDisplay("SPEED UP", "Speed: " + String(motorSpeed));
        break;
        
      case '-':
        motorSpeed = max(motorSpeed - SPEED_STEP, MIN_SPEED);
        setMotorSpeed(motorSpeed);
        updateDisplay("SPEED DOWN", "Speed: " + String(motorSpeed));
        break;
        
      case 'T':
      case 't':
        runFullTest();
        break;
        
      default:
        Serial.println("Unknown command");
        break;
    }
  }
  
  delay(10);
}

void setupMotors() {
  // Configure all motor control pins as outputs
  pinMode(L1_IN1, OUTPUT);
  pinMode(L1_IN2, OUTPUT);
  pinMode(L1_IN3, OUTPUT);
  pinMode(L1_IN4, OUTPUT);
  
  pinMode(R2_IN1, OUTPUT);
  pinMode(R2_IN2, OUTPUT);
  pinMode(R2_IN3, OUTPUT);
  pinMode(R2_IN4, OUTPUT);
  
  // Setup PWM channels
  ledcSetup(PWM_CH_L1, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_L2, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_R1, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_R2, PWM_FREQ, PWM_RES);
  
  // Attach PWM channels to pins
  ledcAttachPin(L1_ENA, PWM_CH_L1);
  ledcAttachPin(L1_ENB, PWM_CH_L2);
  ledcAttachPin(R2_ENA, PWM_CH_R1);
  ledcAttachPin(R2_ENB, PWM_CH_R2);
  
  // Initialize motors stopped
  stopMotors();
  setMotorSpeed(motorSpeed);
  
  Serial.println("Motors initialized");
}

void setupDisplay() {
  tft.begin();
  tft.setRotation(1); // Landscape mode
  tft.fillScreen(COLOR_BG);
  
  // Draw title bar
  tft.fillRect(0, 0, 320, 30, ILI9341_BLUE);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(2);
  tft.setCursor(40, 8);
  tft.println("ESP32 4WD ROBOT");
  
  // Draw separator
  tft.drawLine(0, 30, 320, 30, COLOR_TEXT);
  
  Serial.println("Display initialized");
}

void stopMotors() {
  // Stop all motors
  digitalWrite(L1_IN1, LOW);
  digitalWrite(L1_IN2, LOW);
  digitalWrite(L1_IN3, LOW);
  digitalWrite(L1_IN4, LOW);
  
  digitalWrite(R2_IN1, LOW);
  digitalWrite(R2_IN2, LOW);
  digitalWrite(R2_IN3, LOW);
  digitalWrite(R2_IN4, LOW);
}

void moveForward() {
  // Left motors forward
  digitalWrite(L1_IN1, HIGH);
  digitalWrite(L1_IN2, LOW);
  digitalWrite(L1_IN3, HIGH);
  digitalWrite(L1_IN4, LOW);
  
  // Right motors forward
  digitalWrite(R2_IN1, HIGH);
  digitalWrite(R2_IN2, LOW);
  digitalWrite(R2_IN3, HIGH);
  digitalWrite(R2_IN4, LOW);
}

void moveBackward() {
  // Left motors backward
  digitalWrite(L1_IN1, LOW);
  digitalWrite(L1_IN2, HIGH);
  digitalWrite(L1_IN3, LOW);
  digitalWrite(L1_IN4, HIGH);
  
  // Right motors backward
  digitalWrite(R2_IN1, LOW);
  digitalWrite(R2_IN2, HIGH);
  digitalWrite(R2_IN3, LOW);
  digitalWrite(R2_IN4, HIGH);
}

void turnLeft() {
  // Left motors slow/stop
  ledcWrite(PWM_CH_L1, motorSpeed / 2);
  ledcWrite(PWM_CH_L2, motorSpeed / 2);
  digitalWrite(L1_IN1, HIGH);
  digitalWrite(L1_IN2, LOW);
  digitalWrite(L1_IN3, HIGH);
  digitalWrite(L1_IN4, LOW);
  
  // Right motors full speed forward
  ledcWrite(PWM_CH_R1, motorSpeed);
  ledcWrite(PWM_CH_R2, motorSpeed);
  digitalWrite(R2_IN1, HIGH);
  digitalWrite(R2_IN2, LOW);
  digitalWrite(R2_IN3, HIGH);
  digitalWrite(R2_IN4, LOW);
}

void turnRight() {
  // Left motors full speed forward
  ledcWrite(PWM_CH_L1, motorSpeed);
  ledcWrite(PWM_CH_L2, motorSpeed);
  digitalWrite(L1_IN1, HIGH);
  digitalWrite(L1_IN2, LOW);
  digitalWrite(L1_IN3, HIGH);
  digitalWrite(L1_IN4, LOW);
  
  // Right motors slow/stop
  ledcWrite(PWM_CH_R1, motorSpeed / 2);
  ledcWrite(PWM_CH_R2, motorSpeed / 2);
  digitalWrite(R2_IN1, HIGH);
  digitalWrite(R2_IN2, LOW);
  digitalWrite(R2_IN3, HIGH);
  digitalWrite(R2_IN4, LOW);
}

void spinLeft() {
  // Left motors backward
  digitalWrite(L1_IN1, LOW);
  digitalWrite(L1_IN2, HIGH);
  digitalWrite(L1_IN3, LOW);
  digitalWrite(L1_IN4, HIGH);
  
  // Right motors forward
  digitalWrite(R2_IN1, HIGH);
  digitalWrite(R2_IN2, LOW);
  digitalWrite(R2_IN3, HIGH);
  digitalWrite(R2_IN4, LOW);
}

void spinRight() {
  // Left motors forward
  digitalWrite(L1_IN1, HIGH);
  digitalWrite(L1_IN2, LOW);
  digitalWrite(L1_IN3, HIGH);
  digitalWrite(L1_IN4, LOW);
  
  // Right motors backward
  digitalWrite(R2_IN1, LOW);
  digitalWrite(R2_IN2, HIGH);
  digitalWrite(R2_IN3, LOW);
  digitalWrite(R2_IN4, HIGH);
}

void setMotorSpeed(int speed) {
  ledcWrite(PWM_CH_L1, speed);
  ledcWrite(PWM_CH_L2, speed);
  ledcWrite(PWM_CH_R1, speed);
  ledcWrite(PWM_CH_R2, speed);
}

void updateDisplay(String cmd, String status) {
  // Clear command area
  tft.fillRect(0, 40, 320, 80, COLOR_BG);
  
  // Display command
  tft.setTextSize(3);
  tft.setTextColor(COLOR_FWD);
  tft.setCursor(10, 50);
  tft.println(cmd);
  
  // Display status
  tft.setTextSize(2);
  tft.setTextColor(COLOR_TEXT);
  tft.setCursor(10, 90);
  tft.println(status);
  
  // Display speed
  tft.fillRect(0, 130, 320, 30, COLOR_BG);
  tft.setCursor(10, 135);
  tft.print("Speed: ");
  tft.print(motorSpeed);
  tft.print("/255");
  
  // Draw speed bar
  int barWidth = map(motorSpeed, 0, 255, 0, 280);
  tft.fillRect(20, 170, 280, 20, COLOR_BG);
  tft.drawRect(20, 170, 280, 20, COLOR_TEXT);
  tft.fillRect(21, 171, barWidth, 18, COLOR_FWD);
}

void runFullTest() {
  updateDisplay("TEST MODE", "Starting tests...");
  SerialBT.println("=== FULL SYSTEM TEST ===");
  delay(2000);
  
  // Test 1: Forward
  SerialBT.println("Test 1: Forward");
  updateDisplay("TEST: FORWARD", "All motors FWD");
  moveForward();
  delay(2000);
  stopMotors();
  delay(1000);
  
  // Test 2: Backward
  SerialBT.println("Test 2: Backward");
  updateDisplay("TEST: BACKWARD", "All motors REV");
  moveBackward();
  delay(2000);
  stopMotors();
  delay(1000);
  
  // Test 3: Spin Left
  SerialBT.println("Test 3: Spin Left");
  updateDisplay("TEST: SPIN LEFT", "Counter-clockwise");
  spinLeft();
  delay(2000);
  stopMotors();
  delay(1000);
  
  // Test 4: Spin Right
  SerialBT.println("Test 4: Spin Right");
  updateDisplay("TEST: SPIN RIGHT", "Clockwise");
  spinRight();
  delay(2000);
  stopMotors();
  delay(1000);
  
  // Test 5: Speed Test
  SerialBT.println("Test 5: Speed Ramp");
  for(int spd = MIN_SPEED; spd <= MAX_SPEED; spd += 25) {
    setMotorSpeed(spd);
    updateDisplay("TEST: SPEED", "Speed: " + String(spd));
    moveForward();
    delay(500);
  }
  stopMotors();
  setMotorSpeed(motorSpeed);
  delay(1000);
  
  // Test Complete
  SerialBT.println("=== TEST COMPLETE ===");
  updateDisplay("TEST DONE", "All systems OK!");
  delay(2000);
  updateDisplay("READY", "Waiting for cmd...");
}
