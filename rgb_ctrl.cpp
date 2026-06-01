#include "rgb_ctrl.h"
#include "hardware.h"

LightMode currentLightMode = LIGHT_OFF;
const char* LIGHT_MODE_NAMES[] = {
  "[0] Leds Off",
  "[1] Solid Red",
  "[2] Solid Green",
  "[3] Solid Blue",
  "[4] Breathing Green",
  "[5] Police Strobe",
  "[6] Rainbow Cycle"
};

unsigned long rgbLastUpdate = 0;
float rgbAngle = 0;
bool policeToggle = false;

void updateRGBPattern() {
  unsigned long now = millis();
  
  switch(currentLightMode) {
    case LIGHT_OFF:
      setRGBColor(0, 0, 0);
      break;
      
    case LIGHT_STATIC_RED:
      setRGBColor(255, 0, 0);
      break;
      
    case LIGHT_STATIC_GREEN:
      setRGBColor(0, 255, 0);
      break;
      
    case LIGHT_STATIC_BLUE:
      setRGBColor(0, 0, 255);
      break;
      
    case LIGHT_BREATHING:
      if (now - rgbLastUpdate > 20) {
        rgbLastUpdate = now;
        rgbAngle += 0.05;
        int level = (sin(rgbAngle) + 1.0) * 127.5;
        setRGBColor(0, level, 0);
      }
      break;
      
    case LIGHT_POLICE:
      if (now - rgbLastUpdate > 150) {
        rgbLastUpdate = now;
        policeToggle = !policeToggle;
        if (policeToggle) {
          setRGBColor(255, 0, 0);
        } else {
          setRGBColor(0, 0, 255);
        }
      }
      break;
      
    case LIGHT_RAINBOW:
      if (now - rgbLastUpdate > 30) {
        rgbLastUpdate = now;
        rgbAngle += 0.03;
        int r = (sin(rgbAngle) + 1.0) * 127.5;
        int g = (sin(rgbAngle + 2.094) + 1.0) * 127.5;
        int b = (sin(rgbAngle + 4.188) + 1.0) * 127.5;
        setRGBColor(r, g, b);
      }
      break;
      
    default:
      break;
  }
}

void drawRGBMenu(Adafruit_SH1106G &display, int selectedIdx) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("RGB FX CONTROLLER");
  display.drawFastHLine(0, 9, 128, SH110X_WHITE);

  int y = 12;
  for (int i = 0; i < LIGHT_MODE_COUNT; i++) {
    if (i == selectedIdx) {
      display.fillRect(0, y, 128, 11, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    } else {
      display.setTextColor(SH110X_WHITE);
    }
    
    display.setCursor(4, y + 2);
    display.print(LIGHT_MODE_NAMES[i]);
    
    if (i == (int)currentLightMode) {
      display.setCursor(115, y + 2);
      display.print("<*");
    }
    
    y += 12;
    if (y > 54) break;
  }
  
  display.display();
}
