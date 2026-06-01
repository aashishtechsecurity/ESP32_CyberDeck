#ifndef MATRIX_BOOT_H
#define MATRIX_BOOT_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "config.h"
#include "hardware.h"

struct MatrixDrop {
  int x;
  int y;
  int speed;
  char lastChar;
  bool active;
};

const int NUM_DROPS = 12;
MatrixDrop drops[NUM_DROPS];

void initMatrix(Adafruit_SH1106G &display) {
  for (int i = 0; i < NUM_DROPS; i++) {
    drops[i].x = random(0, SCREEN_WIDTH / 8) * 8;
    drops[i].y = random(-64, 0);
    drops[i].speed = random(3, 7);
    drops[i].lastChar = random(33, 126);
    drops[i].active = true;
  }
}

// Dissolve updates: slows speed and deactivates drops
void drawMatrixDissolveFrame(Adafruit_SH1106G &display, bool dissolve) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  int activeCount = 0;
  for (int i = 0; i < NUM_DROPS; i++) {
    if (!drops[i].active) continue;
    activeCount++;

    display.setCursor(drops[i].x, drops[i].y);
    display.print(drops[i].lastChar);
    
    if (drops[i].y > 8) {
      display.setCursor(drops[i].x, drops[i].y - 8);
      display.print((char)random(33, 126));
    }

    drops[i].y += drops[i].speed;
    drops[i].lastChar = random(33, 126);

    if (drops[i].y > SCREEN_HEIGHT) {
      if (dissolve) {
        drops[i].active = false; // Dissolve by turning off drops when they hit bottom
      } else {
        drops[i].y = random(-20, 0);
        drops[i].speed = random(2, 5);
        drops[i].x = random(0, SCREEN_WIDTH / 8) * 8;
      }
    }
  }
  display.display();
}

void playBootSequence(Adafruit_SH1106G &display) {
  initMatrix(display);

  // --- 1. Matrix Rain & Dissolve Transition ---
  // Pulse Blue LED during rain start
  unsigned long start = millis();
  float angle = 0;
  
  while (millis() - start < 2000) {
    drawMatrixDissolveFrame(display, false);
    int blueVal = (sin(angle) + 1.0) * 127.5;
    setRGBColor(0, 0, blueVal); // Pulsing Blue
    angle += 0.15;
    delay(40);
  }

  // Fade out/Dissolve drops over 1 second
  start = millis();
  while (millis() - start < 1000) {
    drawMatrixDissolveFrame(display, true);
    setRGBColor(0, 0, 50); // Lower blue glow
    delay(40);
  }
  setRGBColor(0, 0, 0);

  // --- 2. Hex Sector Loader Sequence ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Rapidly scroll through simulated hex sectors
  for (int i = 0; i <= 100; i += 4) {
    display.clearDisplay();
    display.setCursor(2, 2);
    display.print("BOOT SECTOR: 0x");
    display.print(random(0x1000, 0x9FFF), HEX);
    
    display.setCursor(2, 16);
    display.print("DECRYPT LOG: ");
    display.print(i);
    display.print("%");

    // Draw high-tech loading progress bar
    display.drawRect(2, 30, 124, 10, SH110X_WHITE);
    display.fillRect(4, 32, (i * 120) / 100, 6, SH110X_WHITE);

    display.setCursor(2, 48);
    display.print("SYSTEM HEALTH: OK");
    display.display();

    // Fast Amber Blink during loading
    if ((i / 4) % 2 == 0) {
      setRGBColor(255, 120, 0); // Amber on
    } else {
      setRGBColor(0, 0, 0); // Amber off
    }
    delay(40);
  }
  setRGBColor(0, 0, 0);
  delay(200);

  // --- 3. Sci-Fi Frame Brackets & 4. Binary Glitch Decryption ---
  // Synced LED: Green Breathing start
  display.clearDisplay();
  
  const char* targetText = "AASHISH OS";
  int len = strlen(targetText);
  char currentRender[16];
  memset(currentRender, ' ', len);
  currentRender[len] = '\0';

  // Glitch characters one by one
  for (int charIdx = 0; charIdx < len; charIdx++) {
    // Flash random binary values '0' or '1' 8 times per character
    for (int glitch = 0; glitch < 6; glitch++) {
      display.clearDisplay();
      
      // Draw Sci-Fi corner brackets
      // Top-Left
      display.drawLine(4, 4, 14, 4, SH110X_WHITE);
      display.drawLine(4, 4, 4, 14, SH110X_WHITE);
      // Top-Right
      display.drawLine(123, 4, 113, 4, SH110X_WHITE);
      display.drawLine(123, 4, 123, 14, SH110X_WHITE);
      // Bottom-Left
      display.drawLine(4, 59, 14, 59, SH110X_WHITE);
      display.drawLine(4, 59, 4, 49, SH110X_WHITE);
      // Bottom-Right
      display.drawLine(123, 59, 113, 59, SH110X_WHITE);
      display.drawLine(123, 59, 123, 49, SH110X_WHITE);

      // Status tags
      display.setTextSize(1);
      display.setCursor(18, 6);
      display.print("SYS: OK");
      display.setCursor(76, 6);
      display.print("PWR: 3.3V");

      // Set glitch character
      currentRender[charIdx] = random(0, 2) ? '1' : '0';

      // Draw active decryption string
      display.setTextSize(2);
      display.setCursor(64 - (len * 12) / 2, 28);
      display.print(currentRender);
      display.display();
      
      setRGBColor(0, random(50, 200), 0); // Green glitch flash
      delay(30);
    }
    
    // Resolve final correct character
    currentRender[charIdx] = targetText[charIdx];
  }

  // Draw final static boot logo screen
  display.clearDisplay();
  // Draw final brackets
  display.drawLine(4, 4, 14, 4, SH110X_WHITE);
  display.drawLine(4, 4, 4, 14, SH110X_WHITE);
  display.drawLine(123, 4, 113, 4, SH110X_WHITE);
  display.drawLine(123, 4, 123, 14, SH110X_WHITE);
  display.drawLine(4, 59, 14, 59, SH110X_WHITE);
  display.drawLine(4, 59, 4, 49, SH110X_WHITE);
  display.drawLine(123, 59, 113, 59, SH110X_WHITE);
  display.drawLine(123, 59, 123, 49, SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(18, 6);
  display.print("SYS: OK");
  display.setCursor(76, 6);
  display.print("PWR: 3.3V");

  display.setTextSize(2);
  display.setCursor(64 - (len * 12) / 2, 28);
  display.print(targetText);
  display.display();

  // Solid green flash for boot complete
  setRGBColor(0, 255, 0);
  delay(1200);
  setRGBColor(0, 0, 0);
}

#endif // MATRIX_BOOT_H
