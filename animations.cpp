#include "animations.h"
#include "config.h"
#include "hardware.h"

// --- Bike Riding States ---
int bikeFrame = 0;
int groundOffset = 0;
float wheelAngle = 0;

struct SmokePuff {
  float x, y;
  float size;
  float speedX;
  float speedY;
  bool active;
};
const int MAX_PUFFS = 6;
SmokePuff puffs[MAX_PUFFS];
unsigned long lastPuffSpawn = 0;

void updateSmoke() {
  if (millis() - lastPuffSpawn > 400) {
    lastPuffSpawn = millis();
    for (int i = 0; i < MAX_PUFFS; i++) {
      if (!puffs[i].active) {
        puffs[i].x = 24.0;
        puffs[i].y = 44.0;
        puffs[i].size = 1.0;
        puffs[i].speedX = -random(8, 15) / 10.0;
        puffs[i].speedY = -random(2, 6) / 10.0;
        puffs[i].active = true;
        break;
      }
    }
  }

  for (int i = 0; i < MAX_PUFFS; i++) {
    if (puffs[i].active) {
      puffs[i].x += puffs[i].speedX;
      puffs[i].y += puffs[i].speedY;
      puffs[i].size += 0.15;
      
      if (puffs[i].size > 6.0 || puffs[i].x < 0) {
        puffs[i].active = false;
      }
    }
  }
}

void drawBikeRiding(Adafruit_SH1106G &display) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(2, 2);
  display.print("BIKE RIDER");

  wheelAngle += 0.35;
  updateSmoke();

  display.drawFastHLine(0, 52, 128, SH110X_WHITE);
  
  groundOffset = (groundOffset + 6) % 24;
  for (int x = 128 - groundOffset; x > -20; x -= 24) {
    display.drawFastHLine(x, 54, 8, SH110X_WHITE);
  }

  static int mountOffset = 0;
  mountOffset = (mountOffset + 1) % 128;
  for (int m = -60; m < 128; m += 80) {
    int mx = m - mountOffset;
    display.drawTriangle(mx + 40, 52, mx + 60, 30, mx + 80, 52, SH110X_WHITE);
  }

  int by = 38 + (bikeFrame % 2); 
  bikeFrame++;

  for (int i = 0; i < MAX_PUFFS; i++) {
    if (puffs[i].active) {
      display.drawCircle(puffs[i].x, puffs[i].y + (by - 38), (int)puffs[i].size, SH110X_WHITE);
    }
  }

  int rx1 = 32;
  int ry1 = by + 8;
  display.drawCircle(rx1, ry1, 6, SH110X_WHITE);
  display.drawCircle(rx1, ry1, 2, SH110X_WHITE);
  display.drawLine(rx1 - 6 * cos(wheelAngle), ry1 - 6 * sin(wheelAngle), rx1 + 6 * cos(wheelAngle), ry1 + 6 * sin(wheelAngle), SH110X_WHITE);
  display.drawLine(rx1 - 6 * cos(wheelAngle + 1.57), ry1 - 6 * sin(wheelAngle + 1.57), rx1 + 6 * cos(wheelAngle + 1.57), ry1 + 6 * sin(wheelAngle + 1.57), SH110X_WHITE);

  int rx2 = 58;
  int ry2 = by + 8;
  display.drawCircle(rx2, ry2, 6, SH110X_WHITE);
  display.drawCircle(rx2, ry2, 2, SH110X_WHITE);
  display.drawLine(rx2 - 6 * cos(wheelAngle), ry2 - 6 * sin(wheelAngle), rx2 + 6 * cos(wheelAngle), ry2 + 6 * sin(wheelAngle), SH110X_WHITE);
  display.drawLine(rx2 - 6 * cos(wheelAngle + 1.57), ry2 - 6 * sin(wheelAngle + 1.57), rx2 + 6 * cos(wheelAngle + 1.57), ry2 + 6 * sin(wheelAngle + 1.57), SH110X_WHITE);

  display.drawLine(32, by + 8, 42, by + 8, SH110X_WHITE); 
  display.drawLine(32, by + 8, 40, by, SH110X_WHITE);     
  display.drawLine(42, by + 8, 40, by, SH110X_WHITE);     
  display.drawLine(42, by + 8, 52, by, SH110X_WHITE);     
  display.drawLine(40, by, 52, by, SH110X_WHITE);         
  display.drawLine(58, by + 8, 54, by - 4, SH110X_WHITE); 
  display.drawLine(54, by - 4, 50, by - 4, SH110X_WHITE); 

  int riderX = 40;
  int riderY = by - 8;
  display.drawCircle(riderX, riderY, 3, SH110X_WHITE);
  display.drawLine(riderX, riderY + 3, riderX, riderY + 12, SH110X_WHITE);
  display.drawLine(riderX, riderY + 12, 34, by + 6, SH110X_WHITE);
  display.drawLine(riderX, riderY + 12, 42, by + 6, SH110X_WHITE);
  display.drawLine(riderX, riderY + 5, 50, by - 2, SH110X_WHITE);

  display.display();
}

// --- Girl & Flowers States ---
const int MAX_PETALS = 12;
struct Petal {
  float x, y;
  float speedX, speedY;
  float windPhase;
};
Petal petals[MAX_PETALS];
bool petalsInit = false;

void initPetals() {
  for (int i = 0; i < MAX_PETALS; i++) {
    petals[i].x = random(40, 128);
    petals[i].y = random(-20, 30);
    petals[i].speedX = -random(5, 15) / 10.0;
    petals[i].speedY = random(8, 15) / 10.0;
    petals[i].windPhase = random(0, 314) / 100.0;
  }
  petalsInit = true;
}

void drawGirlFlowers(Adafruit_SH1106G &display) {
  if (!petalsInit) {
    initPetals();
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(2, 2);
  display.print("GIRL & FLOWERS");

  display.drawFastHLine(0, 56, 128, SH110X_WHITE);

  float breathing = sin(millis() * 0.003) * 0.6;
  int gx = 24;
  int gy = 44;
  
  display.drawCircle(gx, gy - 6 + breathing, 4, SH110X_WHITE);
  display.drawLine(gx, gy - 2 + breathing, gx, gy + 8, SH110X_WHITE);
  display.drawLine(gx, gy + 8, gx + 8, gy + 12, SH110X_WHITE);
  display.drawLine(gx, gy + 8, gx - 4, gy + 12, SH110X_WHITE);
  display.drawLine(gx, gy + 2 + breathing, gx + 8, gy + 4, SH110X_WHITE);

  float sway = sin(millis() * 0.002) * 1.5;
  int fx = 48;
  int fy = 46;
  
  display.drawLine(fx, 56, fx + (int)sway, fy, SH110X_WHITE); 
  int centerColX = fx + (int)sway;
  display.drawCircle(centerColX, fy - 2, 2, SH110X_WHITE);
  display.drawCircle(centerColX - 3, fy - 2, 2, SH110X_WHITE);
  display.drawCircle(centerColX + 3, fy - 2, 2, SH110X_WHITE);
  display.drawCircle(centerColX, fy - 5, 2, SH110X_WHITE);
  display.drawCircle(centerColX, fy + 1, 2, SH110X_WHITE);

  for (int i = 0; i < MAX_PETALS; i++) {
    float flutter = sin(millis() * 0.006 + petals[i].windPhase) * 2.0;
    int px = petals[i].x + flutter;
    int py = petals[i].y;

    if (px >= 0 && px < 128 && py >= 0 && py < 64) {
      display.drawPixel(px, py, SH110X_WHITE);
      if (i % 2 == 0) {
        display.drawPixel(px + 1, py, SH110X_WHITE);
        display.drawPixel(px, py + 1, SH110X_WHITE);
      }
    }
    
    petals[i].x += petals[i].speedX;
    petals[i].y += petals[i].speedY;

    if (petals[i].y > 55 || petals[i].x < 0) {
      petals[i].x = random(40, 128);
      petals[i].y = random(-10, 10);
    }
  }

  display.display();
}
