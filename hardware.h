#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include "config.h"

enum JoyDirection {
  JOY_NONE,
  JOY_UP,
  JOY_DOWN,
  JOY_LEFT,
  JOY_RIGHT,
  JOY_CLICK
};

// Global calibration variables
int joyCenterX = 1850;
int joyCenterY = 1850;

// Tracks button state to detect single-click transitions
bool lastButtonState = HIGH;
unsigned long lastButtonDebounce = 0;
const unsigned long DEBOUNCE_DELAY = 50;

// Track if joystick axis has returned to center before registering another movement
bool axisReleased = true;

// Track last user activity time for power management/sleep
unsigned long lastActivityTime = 0;

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

  lastActivityTime = millis();
}

void calibrateJoystick() {
  // Read joystick multiple times at setup to set center offsets
  long sumX = 0;
  long sumY = 0;
  const int samples = 10;
  
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
  // Read button
  int btnVal = digitalRead(PIN_JOY_BTN);
  if (btnVal == LOW && lastButtonState == HIGH) {
    if (millis() - lastButtonDebounce > DEBOUNCE_DELAY) {
      lastButtonDebounce = millis();
      lastButtonState = LOW;
      lastActivityTime = millis(); // Update activity timer
      return JOY_CLICK;
    }
  }
  if (btnVal == HIGH) {
    lastButtonState = HIGH;
  }

  // Read analog stick values
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
      lastActivityTime = millis(); // Update activity timer
      return JOY_LEFT;
    }
    if (xVal > joyCenterX + JOY_THRESHOLD) {
      axisReleased = false;
      lastActivityTime = millis(); // Update activity timer
      return JOY_RIGHT;
    }
    if (yVal < joyCenterY - JOY_THRESHOLD) {
      axisReleased = false;
      lastActivityTime = millis(); // Update activity timer
      return JOY_UP;
    }
    if (yVal > joyCenterY + JOY_THRESHOLD) {
      axisReleased = false;
      lastActivityTime = millis(); // Update activity timer
      return JOY_DOWN;
    }
  }

  return JOY_NONE;
}

#endif // HARDWARE_H
