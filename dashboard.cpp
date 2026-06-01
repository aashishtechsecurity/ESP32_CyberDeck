#include "dashboard.h"
#include "config.h"

const int GRAPH_LEN = 40;
int cpuHistory[GRAPH_LEN] = {0};
int memHistory[GRAPH_LEN] = {0};
int graphHead = 0;

void initDashboard() {
  for (int i = 0; i < GRAPH_LEN; i++) {
    cpuHistory[i] = random(10, 40);
    memHistory[i] = random(20, 50);
  }
}

void updateDashboardData() {
  int lastCpu = cpuHistory[(graphHead + GRAPH_LEN - 1) % GRAPH_LEN];
  int nextCpu = lastCpu + random(-8, 9);
  if (nextCpu < 5) nextCpu = 5;
  if (nextCpu > 55) nextCpu = 55;
  
  int lastMem = memHistory[(graphHead + GRAPH_LEN - 1) % GRAPH_LEN];
  int nextMem = lastMem + random(-4, 5);
  if (nextMem < 10) nextMem = 10;
  if (nextMem > 55) nextMem = 55;

  cpuHistory[graphHead] = nextCpu;
  memHistory[graphHead] = nextMem;
  graphHead = (graphHead + 1) % GRAPH_LEN;
}

void drawDashboard(Adafruit_SH1106G &display) {
  display.clearDisplay();
  display.drawRect(0, 0, 128, 64, SH110X_WHITE);
  display.drawFastVLine(64, 0, 64, SH110X_WHITE);
  display.drawFastHLine(0, 12, 128, SH110X_WHITE);

  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(4, 2);
  display.print("SYS_MON");

  display.setCursor(72, 2);
  display.print("NET_LOG");

  display.setCursor(4, 15);
  display.print("CPU:");
  int currentCpu = cpuHistory[(graphHead + GRAPH_LEN - 1) % GRAPH_LEN];
  display.print(currentCpu);
  display.print("%");

  display.drawRect(4, 26, GRAPH_LEN + 2, 28, SH110X_WHITE);
  
  for (int i = 0; i < GRAPH_LEN - 1; i++) {
    int valIdx1 = (graphHead + i) % GRAPH_LEN;
    int valIdx2 = (valIdx1 + 1) % GRAPH_LEN;
    
    int y1 = 53 - (cpuHistory[valIdx1] * 24 / 100);
    int y2 = 53 - (cpuHistory[valIdx2] * 24 / 100);
    display.drawLine(5 + i, y1, 5 + i + 1, y2, SH110X_WHITE);
  }

  static unsigned long lastLogChange = 0;
  static int logPhase = 0;
  
  if (millis() - lastLogChange > 1200) {
    lastLogChange = millis();
    logPhase = (logPhase + 1) % 4;
  }

  display.setCursor(68, 15);
  display.print("IP:192.168.");
  display.print((1 + logPhase * 3));
  display.print(".");
  display.print((42 + logPhase * 11));

  display.setCursor(68, 26);
  if (logPhase == 0) {
    display.print("SQL INJECT");
  } else if (logPhase == 1) {
    display.print("PORT SCAN..");
  } else if (logPhase == 2) {
    display.print("DDOS SENT");
  } else {
    display.print("SHELL SPWN");
  }

  display.fillRect(68, 38, 56, 20, SH110X_WHITE);
  display.setTextColor(SH110X_BLACK);
  display.setCursor(72, 44);
  display.print("GRANTED");

  display.display();
}
