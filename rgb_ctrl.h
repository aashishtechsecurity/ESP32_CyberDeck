#ifndef RGB_CTRL_H
#define RGB_CTRL_H

#include <Adafruit_SH110X.h>

enum LightMode {
  LIGHT_OFF,
  LIGHT_STATIC_RED,
  LIGHT_STATIC_GREEN,
  LIGHT_STATIC_BLUE,
  LIGHT_BREATHING,
  LIGHT_POLICE,
  LIGHT_RAINBOW,
  LIGHT_MODE_COUNT
};

extern LightMode currentLightMode;
extern const char* LIGHT_MODE_NAMES[];

void updateRGBPattern();
void drawRGBMenu(Adafruit_SH1106G &display, int selectedIdx);

#endif // RGB_CTRL_H
