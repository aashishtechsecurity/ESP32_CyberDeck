#ifndef CONFIG_H
#define CONFIG_H

// --- Hardware Pins ---
#define PIN_RGB_RED    2
#define PIN_RGB_BLUE   4
#define PIN_RGB_GREEN  15

#define PIN_OLED_SDA   21
#define PIN_OLED_SCL   22

#define PIN_JOY_BTN    25
#define PIN_JOY_X      34
#define PIN_JOY_Y      35

// --- OLED Screen Settings ---
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_ADDRESS   0x3C

// --- Joystick Calibration (Auto-calibrated at boot) ---
extern int joyCenterX;
extern int joyCenterY;
#define JOY_THRESHOLD  800  // Sensitivity threshold for directional movements

#endif // CONFIG_H
