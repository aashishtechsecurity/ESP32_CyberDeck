#include "hardware.h"
#include "config.h"

#if __has_include(<esp_arduino_version.h>)
#include <esp_arduino_version.h>
#endif

constexpr uint8_t RGB_RED_CH = 0;   // used by ESP32 core 2.x API
constexpr uint8_t RGB_GREEN_CH = 1; // used by ESP32 core 2.x API
constexpr uint8_t RGB_BLUE_CH = 2;  // used by ESP32 core 2.x API
constexpr uint32_t RGB_PWM_FREQ = 5000;
constexpr uint8_t RGB_PWM_RES_BITS = 8;

// Define global variables
int joyCenterX = 1850;
int joyCenterY = 1850;
unsigned long lastActivityTime = 0;
volatile bool buttonInterruptFlag = false;

const unsigned long INTERRUPT_DEBOUNCE_DELAY = 180; // ms
unsigned long lastButtonHandledTime = 0;

// Track if joystick axis has returned to center before registering another movement
bool axisReleased = true;

// Interrupt Service Routine (ISR) for the Joystick Button Click
void IRAM_ATTR buttonISR() {
  buttonInterruptFlag = true;
}

void setupHardware() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(PIN_RGB_RED, RGB_PWM_FREQ, RGB_PWM_RES_BITS);
  ledcAttach(PIN_RGB_GREEN, RGB_PWM_FREQ, RGB_PWM_RES_BITS);
  ledcAttach(PIN_RGB_BLUE, RGB_PWM_FREQ, RGB_PWM_RES_BITS);
#else
  ledcSetup(RGB_RED_CH, RGB_PWM_FREQ, RGB_PWM_RES_BITS);
  ledcSetup(RGB_GREEN_CH, RGB_PWM_FREQ, RGB_PWM_RES_BITS);
  ledcSetup(RGB_BLUE_CH, RGB_PWM_FREQ, RGB_PWM_RES_BITS);
  ledcAttachPin(PIN_RGB_RED, RGB_RED_CH);
  ledcAttachPin(PIN_RGB_GREEN, RGB_GREEN_CH);
  ledcAttachPin(PIN_RGB_BLUE, RGB_BLUE_CH);
#endif
  
  pinMode(PIN_JOY_BTN, INPUT_PULLUP);
  pinMode(PIN_JOY_X, INPUT);
  pinMode(PIN_JOY_Y, INPUT);
  
  // Turn off RGB initially
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(PIN_RGB_RED, 0);
  ledcWrite(PIN_RGB_GREEN, 0);
  ledcWrite(PIN_RGB_BLUE, 0);
#else
  ledcWrite(RGB_RED_CH, 0);
  ledcWrite(RGB_GREEN_CH, 0);
  ledcWrite(RGB_BLUE_CH, 0);
#endif

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
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(PIN_RGB_RED, r);
  ledcWrite(PIN_RGB_GREEN, g);
  ledcWrite(PIN_RGB_BLUE, b);
#else
  ledcWrite(RGB_RED_CH, r);
  ledcWrite(RGB_GREEN_CH, g);
  ledcWrite(RGB_BLUE_CH, b);
#endif
}

JoyDirection readJoystick() {
  // Check if interrupt flag was tripped
  if (buttonInterruptFlag) {
    unsigned long now = millis();
    if (now - lastButtonHandledTime > INTERRUPT_DEBOUNCE_DELAY) {
      lastButtonHandledTime = now;
      buttonInterruptFlag = false; // Reset flag after accepting click
      lastActivityTime = now;
      return JOY_CLICK;
    }
    buttonInterruptFlag = false; // Ignore bounces
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
