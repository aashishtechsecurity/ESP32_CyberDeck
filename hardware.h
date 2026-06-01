#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>

enum JoyDirection {
  JOY_NONE,
  JOY_UP,
  JOY_DOWN,
  JOY_LEFT,
  JOY_RIGHT,
  JOY_CLICK
};

// Global variables shared across components
extern int joyCenterX;
extern int joyCenterY;
extern unsigned long lastActivityTime;

// Hardware Driver APIs
void setupHardware();
void calibrateJoystick();
void setRGBColor(uint8_t r, uint8_t g, uint8_t b);
JoyDirection readJoystick();

#endif // HARDWARE_H
