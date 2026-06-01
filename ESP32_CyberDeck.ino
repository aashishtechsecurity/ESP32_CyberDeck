#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <esp_task_wdt.h> // ESP32 Hardware Task Watchdog

#include "config.h"
#include "hardware.h"
#include "matrix_boot.h"
#include "scanner.h"
#include "dashboard.h"
#include "games.h"
#include "rgb_ctrl.h"
#include "animations.h"

// Instantiate the modern Adafruit SH1106G display
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// System UI States
enum OSState {
  STATE_BOOT,
  STATE_WIFI_SCAN,
  STATE_BLE_SCAN,
  STATE_GAME,
  STATE_DASHBOARD,
  STATE_RGB_CTRL,
  STATE_SYS_INFO,
  STATE_ANIMATIONS,
  STATE_ABOUT,
  STATE_COUNT
};

OSState currentState = STATE_WIFI_SCAN;
bool appActive = false;

// Idle Screensaver tracking variables
bool isIdleScreensaver = false;
OSState savedPreIdleState = STATE_WIFI_SCAN;
bool savedPreIdleAppActive = false;
unsigned long lastIdleAnimSwitch = 0;
int idleAnimCycle = 0;

// Sub-app variables
int subAppSelectIdx = 0;
unsigned long lastGameTick = 0;
unsigned long lastDashTick = 0;

// --- Screensaver Animation Variables ---
int animType = 0;
const int MAX_STARS = 30;
struct Star {
  float x, y, z;
};
Star stars[MAX_STARS];

void initStars() {
  for (int i = 0; i < MAX_STARS; i++) {
    stars[i].x = random(-64, 64);
    stars[i].y = random(-32, 32);
    stars[i].z = random(1, 64);
  }
}

void setup() {
  Serial.begin(115200);
  
  // Setup hardware pins
  setupHardware();

  // Run dynamic calibration of joystick offsets at boot
  calibrateJoystick();

  // Initialize display via I2C at address 0x3C
  if (!display.begin(OLED_ADDRESS, true)) {
    Serial.println("CRITICAL ERROR: SH1106 display not detected on I2C!");
    setRGBColor(255, 0, 0); // Solid RED status light on hardware crash
    while (1); // Halt execution
  }
  
  // Boost I2C bus speed to Fast Mode (400kHz) for fluid UI updates
  Wire.setClock(400000);
  
  display.clearDisplay();
  display.display();

  // Run the boot sequence *before* watchdog is initialized to avoid watchdog reset loops
  playBootSequence(display);

  // Initialize Task Watchdog Timer (4 seconds limit, system panic on reset trigger)
  esp_task_wdt_config_t wdt_config = {
      .timeout_ms = 4000,
      .idle_core_mask = (1 << 0) | (1 << 1), // Monitor both CPU cores
      .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL); // Subscribe main loop task to WDT

  initStars();
  triggerWifiScan(); // Trigger initial WiFi scan for first screen
}

// Draw a beautiful custom icon based on the current state/app
void drawAppIcon(OSState state) {
  display.drawRect(48, 16, 32, 32, SH110X_WHITE);
  
  switch(state) {
    case STATE_WIFI_SCAN:
      display.drawCircle(64, 42, 2, SH110X_WHITE);
      display.drawCircle(64, 42, 6, SH110X_WHITE);
      display.drawCircle(64, 42, 10, SH110X_WHITE);
      break;
      
    case STATE_BLE_SCAN:
      display.drawLine(64, 20, 64, 44, SH110X_WHITE);
      display.drawLine(64, 20, 72, 26, SH110X_WHITE);
      display.drawLine(72, 26, 64, 32, SH110X_WHITE);
      display.drawLine(64, 32, 72, 38, SH110X_WHITE);
      display.drawLine(72, 38, 64, 44, SH110X_WHITE);
      display.drawLine(58, 26, 70, 38, SH110X_WHITE);
      display.drawLine(58, 38, 70, 26, SH110X_WHITE);
      break;
      
    case STATE_GAME:
      display.fillRect(52, 28, 24, 8, SH110X_WHITE);
      display.fillRect(60, 20, 8, 24, SH110X_WHITE);
      break;
      
    case STATE_DASHBOARD:
      display.drawRect(52, 20, 24, 18, SH110X_WHITE);
      display.drawLine(60, 38, 56, 44, SH110X_WHITE);
      display.drawLine(68, 38, 72, 44, SH110X_WHITE);
      display.drawLine(56, 44, 72, 44, SH110X_WHITE);
      break;
      
    case STATE_RGB_CTRL:
      display.drawCircle(64, 28, 8, SH110X_WHITE);
      display.fillRect(61, 36, 6, 6, SH110X_WHITE);
      display.drawLine(64, 18, 64, 16, SH110X_WHITE);
      display.drawLine(54, 28, 52, 28, SH110X_WHITE);
      display.drawLine(74, 28, 76, 28, SH110X_WHITE);
      break;
      
    case STATE_SYS_INFO:
      display.drawRect(54, 22, 20, 20, SH110X_WHITE);
      display.fillRect(58, 26, 12, 12, SH110X_WHITE);
      display.drawFastHLine(50, 26, 4, SH110X_WHITE);
      display.drawFastHLine(50, 32, 4, SH110X_WHITE);
      display.drawFastHLine(50, 38, 4, SH110X_WHITE);
      display.drawFastHLine(74, 26, 4, SH110X_WHITE);
      display.drawFastHLine(74, 32, 4, SH110X_WHITE);
      display.drawFastHLine(74, 38, 4, SH110X_WHITE);
      break;
      
    case STATE_ANIMATIONS:
      display.drawLine(50, 32, 58, 24, SH110X_WHITE);
      display.drawLine(58, 24, 66, 40, SH110X_WHITE);
      display.drawLine(66, 40, 74, 24, SH110X_WHITE);
      display.drawLine(74, 24, 78, 32, SH110X_WHITE);
      break;
      
    case STATE_ABOUT:
      display.setCursor(54, 24);
      display.setTextSize(2);
      display.setTextColor(SH110X_WHITE);
      display.print("</>");
      break;
      
    default:
      break;
  }
}

// Draw the Lobby cover screen card
void drawAppLobby(const char* title, OSState state) {
  display.clearDisplay();
  
  // Upper state index dots
  int totalDots = STATE_COUNT - 1;
  int currentDotIdx = (int)state - 1;
  int startX = 64 - (totalDots * 6) / 2;
  for (int i = 0; i < totalDots; i++) {
    if (i == currentDotIdx) {
      display.fillCircle(startX + i * 6, 6, 2, SH110X_WHITE);
    } else {
      display.drawCircle(startX + i * 6, 6, 2, SH110X_WHITE);
    }
  }

  // App Title
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(64 - (strlen(title) * FONT_WIDTH) / 2, 50);
  display.print(title);
  
  drawAppIcon(state);
  display.display();
}

void renderScreensaver(JoyDirection input) {
  if (input == JOY_CLICK) {
    animType = (animType + 1) % 5;
    display.clearDisplay();
  }

  display.clearDisplay();
  
  if (animType == 0) {
    for (int i = 0; i < MAX_STARS; i++) {
      stars[i].z -= 1.5;
      if (stars[i].z <= 0) {
        stars[i].x = random(-64, 64);
        stars[i].y = random(-32, 32);
        stars[i].z = 64;
      }
      int sx = (stars[i].x * 32) / stars[i].z + 64;
      int sy = (stars[i].y * 32) / stars[i].z + 32;
      if (sx >= 0 && sx < SCREEN_WIDTH && sy >= 0 && sy < SCREEN_HEIGHT) {
        int r = (64 - stars[i].z) / 16;
        display.fillCircle(sx, sy, r > 0 ? r : 1, SH110X_WHITE);
      }
    }
    display.setCursor(UI_PADDING_X, UI_PADDING_Y);
    display.print("FX: Starfield");
    display.display();
  } 
  else if (animType == 1) {
    static float phase = 0;
    phase += 0.15;
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      int y = 32 + sin(x * 0.1 + phase) * 15 + cos(x * 0.05 - phase) * 5;
      display.drawPixel(x, y, SH110X_WHITE);
    }
    display.setCursor(UI_PADDING_X, UI_PADDING_Y);
    display.print("FX: Sinewave");
    display.display();
  } 
  else if (animType == 2) {
    static int offset = 0;
    offset = (offset + 1) % 16;
    for (int i = offset; i < SCREEN_WIDTH; i += 16) {
      display.drawLine(i, 0, i, SCREEN_HEIGHT, SH110X_WHITE);
    }
    for (int i = offset; i < SCREEN_HEIGHT; i += 16) {
      display.drawLine(0, i, SCREEN_WIDTH, i, SH110X_WHITE);
    }
    display.setCursor(UI_PADDING_X, UI_PADDING_Y);
    display.print("FX: Cyber Grid");
    display.display();
  }
  else if (animType == 3) {
    drawBikeRiding(display);
  }
  else if (animType == 4) {
    drawGirlFlowers(display);
  }
}

void loop() {
  // Feed/Reset the hardware Task Watchdog Timer
  esp_task_wdt_reset();

  JoyDirection input = readJoystick();
  static JoyDirection bufferedJoyInput = JOY_NONE;
  if (input != JOY_NONE) {
    bufferedJoyInput = input;
  }

  // --- Idle Screensaver Check (5 Seconds) ---
  if (!isIdleScreensaver && (millis() - lastActivityTime > 5000)) {
    savedPreIdleState = currentState;
    savedPreIdleAppActive = appActive;
    isIdleScreensaver = true;
    lastIdleAnimSwitch = millis();
    idleAnimCycle = 0;
  }

  // Wake up if joystick is touched
  if (isIdleScreensaver && input != JOY_NONE) {
    isIdleScreensaver = false;
    currentState = savedPreIdleState;
    appActive = savedPreIdleAppActive;
    lastActivityTime = millis();
    bufferedJoyInput = JOY_NONE;
    input = JOY_NONE;
  }

  // Render idle screensaver animation if active
  if (isIdleScreensaver) {
    updateRGBPattern();
    
    if (millis() - lastIdleAnimSwitch > 8000) {
      lastIdleAnimSwitch = millis();
      idleAnimCycle = (idleAnimCycle + 1) % 3;
      if (idleAnimCycle == 0) {
        initMatrix(display);
      }
    }

    static unsigned long lastAnimTick = 0;
    if (millis() - lastAnimTick > 50) {
      lastAnimTick = millis();
      if (idleAnimCycle == 0) {
        drawMatrixDissolveFrame(display, false);
      } else if (idleAnimCycle == 1) {
        drawBikeRiding(display);
      } else if (idleAnimCycle == 2) {
        drawGirlFlowers(display);
      }
    }

    // Light sleep after 5 minutes of screensaver
    if (millis() - lastActivityTime > 300000) {
      display.clearDisplay();
      display.display();
      display.oled_command(SH110X_DISPLAYOFF);
      setRGBColor(0, 0, 0);
      esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_JOY_BTN, 0);
      esp_light_sleep_start();
      display.oled_command(SH110X_DISPLAYON);
      lastActivityTime = millis();
      isIdleScreensaver = false;
    }
    return;
  }

  // Standard deep sleep backup (if screensaver config differs)
  if (millis() - lastActivityTime > 300000) {
    display.clearDisplay();
    display.display();
    display.oled_command(SH110X_DISPLAYOFF);
    setRGBColor(0, 0, 0);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_JOY_BTN, 0);
    esp_light_sleep_start();
    display.oled_command(SH110X_DISPLAYON);
    lastActivityTime = millis();
  }

  updateRGBPattern();

  // Navigation Logic
  if (!appActive) {
    if (input == JOY_RIGHT) {
      int nextState = (int)currentState + 1;
      if (nextState >= STATE_COUNT) nextState = STATE_WIFI_SCAN;
      currentState = (OSState)nextState;
    } 
    else if (input == JOY_LEFT) {
      int prevState = (int)currentState - 1;
      if (prevState < (int)STATE_WIFI_SCAN) prevState = STATE_ABOUT;
      currentState = (OSState)prevState;
    } 
    else if (input == JOY_CLICK) {
      appActive = true;
      subAppSelectIdx = 0;
      if (currentState == STATE_WIFI_SCAN) triggerWifiScan();
      if (currentState == STATE_BLE_SCAN) triggerBleScan();
      if (currentState == STATE_GAME) initSnakeGame();
      if (currentState == STATE_DASHBOARD) initDashboard();
    }
  } else {
    if (currentState == STATE_GAME) {
      if (gameOver && input == JOY_LEFT) {
        appActive = false;
      }
    } else {
      if (input == JOY_LEFT) {
        appActive = false;
      }
    }
  }

  // State Renderer
  if (!appActive) {
    switch (currentState) {
      case STATE_WIFI_SCAN:
        drawAppLobby("1. WIFI SCANNER", STATE_WIFI_SCAN);
        break;
      case STATE_BLE_SCAN:
        drawAppLobby("2. BLE SCANNER", STATE_BLE_SCAN);
        break;
      case STATE_GAME:
        drawAppLobby("3. SNAKE GAME", STATE_GAME);
        break;
      case STATE_DASHBOARD:
        drawAppLobby("4. MONITOR DASH", STATE_DASHBOARD);
        break;
      case STATE_RGB_CTRL:
        drawAppLobby("5. RGB CONTROLLER", STATE_RGB_CTRL);
        break;
      case STATE_SYS_INFO:
        drawAppLobby("6. SYSTEM STATUS", STATE_SYS_INFO);
        break;
      case STATE_ANIMATIONS:
        drawAppLobby("7. SCREENSAVERS", STATE_ANIMATIONS);
        break;
      case STATE_ABOUT:
        drawAppLobby("8. ABOUT SYSTEM", STATE_ABOUT);
        break;
      default:
        break;
    }
  } else {
    switch (currentState) {
      case STATE_WIFI_SCAN:
        if (input == JOY_UP) {
          if (!wifiScanRunning && wifiCount > 0) {
            subAppSelectIdx = (subAppSelectIdx - 1 + wifiCount) % wifiCount;
          }
        } else if (input == JOY_DOWN) {
          if (!wifiScanRunning && wifiCount > 0) {
            subAppSelectIdx = (subAppSelectIdx + 1) % wifiCount;
          }
        } else if (input == JOY_CLICK) {
          triggerWifiScan();
          subAppSelectIdx = 0;
        }
        renderWifiList(display, subAppSelectIdx);
        break;

      case STATE_BLE_SCAN:
        if (input == JOY_UP) {
          if (!bleScanRunning && bleCount > 0) {
            subAppSelectIdx = (subAppSelectIdx - 1 + bleCount) % bleCount;
          }
        } else if (input == JOY_DOWN) {
          if (!bleScanRunning && bleCount > 0) {
            subAppSelectIdx = (subAppSelectIdx + 1) % bleCount;
          }
        } else if (input == JOY_CLICK) {
          triggerBleScan();
          subAppSelectIdx = 0;
        }
        renderBleList(display, subAppSelectIdx);
        break;

      case STATE_GAME:
        if (input == JOY_CLICK && gameOver) {
          initSnakeGame();
        }
        if (millis() - lastGameTick > 130) {
          lastGameTick = millis();
          updateSnake(bufferedJoyInput);
          bufferedJoyInput = JOY_NONE;
        }
        drawSnakeGame(display);
        break;

      case STATE_DASHBOARD:
        if (millis() - lastDashTick > 400) {
          lastDashTick = millis();
          updateDashboardData();
        }
        drawDashboard(display);
        break;

      case STATE_RGB_CTRL:
        if (input == JOY_UP) {
          subAppSelectIdx = (subAppSelectIdx - 1 + LIGHT_MODE_COUNT) % LIGHT_MODE_COUNT;
        } else if (input == JOY_DOWN) {
          subAppSelectIdx = (subAppSelectIdx + 1) % LIGHT_MODE_COUNT;
        } else if (input == JOY_CLICK) {
          currentLightMode = (LightMode)subAppSelectIdx;
        }
        drawRGBMenu(display, subAppSelectIdx);
        break;

      case STATE_SYS_INFO:
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(0, 0);
        display.println("SYSTEM STATUS");
        display.drawFastHLine(0, 9, 128, SH110X_WHITE);
        
        display.setCursor(2, 14);
        display.print("Uptime: ");
        display.print(millis() / 1000);
        display.println("s");
        
        display.setCursor(2, 24);
        display.print("Free RAM: ");
        display.print(ESP.getFreeHeap() / 1024);
        display.println(" KB");

        display.setCursor(2, 34);
        display.print("CPU Freq: ");
        display.print(ESP.getCpuFreqMHz());
        display.println(" MHz");

        display.setCursor(2, 44);
        display.print("Temp: ");
        display.print(42.5 + random(-1, 2) * 0.2, 1);
        display.println(" C");
        
        display.display();
        break;

      case STATE_ANIMATIONS:
        renderScreensaver(input);
        break;

      case STATE_ABOUT:
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(22, 4);
        display.println("AASHISH OS v1.0");
        display.drawFastHLine(0, 14, 128, SH110X_WHITE);
        
        display.setCursor(4, 20);
        display.println("DEVELOPER:");
        display.setTextSize(2);
        display.setCursor(22, 32);
        display.println("AASHISH");
        
        display.setTextSize(1);
        static bool cursorBlink = false;
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 500) {
          lastBlink = millis();
          cursorBlink = !cursorBlink;
        }
        display.setCursor(108, 38);
        display.print(cursorBlink ? "_" : " ");
        display.display();
        break;

      default:
        break;
    }
  }
}
