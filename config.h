#ifndef CONFIG_H
#define CONFIG_H

// --- Hardware Pins ---
constexpr int PIN_RGB_RED = 2;
constexpr int PIN_RGB_BLUE = 4;
constexpr int PIN_RGB_GREEN = 15;

constexpr int PIN_OLED_SDA = 21;
constexpr int PIN_OLED_SCL = 22;

constexpr int PIN_JOY_BTN = 25;
constexpr int PIN_JOY_X = 34;
constexpr int PIN_JOY_Y = 35;

// --- OLED Screen Settings ---
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// --- Display Geometry Layout Constants ---
constexpr int UI_PADDING_X = 4;
constexpr int UI_PADDING_Y = 2;
constexpr int FONT_WIDTH = 6;
constexpr int FONT_HEIGHT = 8;

// --- Joystick Calibration (Auto-calibrated at boot) ---
extern int joyCenterX;
extern int joyCenterY;
constexpr int JOY_THRESHOLD = 800; // Sensitivity threshold for directional movements

#endif // CONFIG_H
