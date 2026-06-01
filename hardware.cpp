#include "hardware.h"
#include "config.h"

// Define global variables
int joyCenterX = 1850;
int joyCenterY = 1850;
unsigned long lastActivityTime = 0;
volatile bool buttonClickedFlag = false;

// Local debouncing tracking inside ISR
volatile unsigned long lastButtonInterruptTime = 0;
const unsigned long INTERRUPT_DEBOUNCE_DELAY = 180; // ms

// Track if joystick axis has returned to center before registering another movement
bool axisReleased = true;

// Interrupt Service Routine (ISR) for the Joystick Button Click
void IRAM_ATTR buttonISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastButtonInterruptTime > INTERRUPT_DEBOUNCE_DELAY) {
    lastButtonInterruptTime = currentTime;
    buttonClickedFlag = true;
  }
}

void setupHardware() {
  pinMode(PIN_RGB_RED, OUTPUT);
  pinMode(PIN_RGB_GREEN, OUTPUT);
  pinMode(PIN_RGB_BLUE, OUTPUT);
  
  pinMode(PIN_JOY_BTN, INPUT_PULLUP);
  pinMode(PIN_JOY_X, INPUT);
  pinMode(PIN_JOY_Y, INPUT);
  
  // Turn off RGB initially
  digitalWrite(PIN_RGB_RED, LOW);
  digitalWrite(PIN_RGB_GREEN, LOW);
  digitalWrite(PIN_RGB_BLUE, LOW);

  // Attach Hardware Interrupt for button clicking (GPIO25, Active LOW)
  attachInterrupt(digitalPinToInterrupt(PIN_JOY_BTN), buttonISR, FALLING);

  lastActivityTime = millis();
}

void calibrateJoystick() {
  long sumX = 0;
  long sumY = 0;
  const int samples = 12;
  
  for (int i = 0; i < samples; i++) {
    sumX += analogRead(PIN_JOY_X);
    sumY += analogRead(PIN_JOY_Y);
    delay(20);
  }
  
  joyCenterX = sumX / samples;
  joyCenterY = sumY / samples;
}

void setRGBColor(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(PIN_RGB_RED, r);
  analogWrite(PIN_RGB_GREEN, g);
  analogWrite(PIN_RGB_BLUE, b);
}

JoyDirection readJoystick() {
  // Check if interrupt flag was tripped
  if (buttonClickedFlag) {
    buttonClickedFlag = false; // Reset flag
    lastActivityTime = millis();
    return JOY_CLICK;
  }

  // Read analog values
  int xVal = analogRead(PIN_JOY_X);
  int yVal = analogRead(PIN_JOY_Y);

  // Check if centered to reset latch
  if (abs(xVal - joyCenterX) < JOY_THRESHOLD && abs(yVal - joyCenterY) < JOY_THRESHOLD) {
    axisReleased = true;
    return JOY_NONE;
  }

  // If latch is released, detect movements
  if (axisReleased) {
    if (xVal < joyCenterX - JOY_THRESHOLD) {
      axisReleased = false;
      lastActivityTime = millis();
      return JOY_LEFT;
    }
    if (xVal > joyCenterX + JOY_THRESHOLD) {
      axisReleased = false;
      lastActivityTime = millis();
      return JOY_RIGHT;
    }
    if (yVal < joyCenterY - JOY_THRESHOLD) {
      axisReleased = false;
      lastActivityTime = millis();
      return JOY_UP;
    }
    if (yVal > joyCenterY + JOY_THRESHOLD) {
      axisReleased = false;
      lastActivityTime = millis();
      return JOY_DOWN;
    }
  }

  return JOY_NONE;
}
