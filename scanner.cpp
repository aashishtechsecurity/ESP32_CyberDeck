#include "scanner.h"
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "hardware.h"
#include "config.h"

// Define external variables
WifiNetworkInfo wifiResults[MAX_NETWORKS];
volatile int wifiCount = 0;
volatile bool wifiScanRunning = false;

BleDeviceInfo bleResults[MAX_BLE_DEVICES];
volatile int bleCount = 0;
volatile bool bleScanRunning = false;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (bleCount < MAX_BLE_DEVICES) {
        const char* nameStr = advertisedDevice.haveName() ? advertisedDevice.getName().c_str() : "Unknown";
        const char* addrStr = advertisedDevice.getAddress().toString().c_str();
        int rssi = advertisedDevice.getRSSI();
        
        bool exists = false;
        for(int i = 0; i < bleCount; i++) {
          if(strcmp(bleResults[i].address, addrStr) == 0) {
            exists = true;
            break;
          }
        }

        if(!exists) {
          int idx = bleCount;
          strncpy(bleResults[idx].name, nameStr, sizeof(bleResults[idx].name) - 1);
          bleResults[idx].name[sizeof(bleResults[idx].name) - 1] = '\0';
          
          strncpy(bleResults[idx].address, addrStr, sizeof(bleResults[idx].address) - 1);
          bleResults[idx].address[sizeof(bleResults[idx].address) - 1] = '\0';
          
          bleResults[idx].rssi = rssi;
          bleCount++;
        }
      }
    }
};

void wifiScanTask(void *pvParameters) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();
  int count = (n > MAX_NETWORKS) ? MAX_NETWORKS : n;
  wifiCount = 0;

  for (int i = 0; i < count; i++) {
    String rawSsid = WiFi.SSID(i);
    strncpy(wifiResults[i].ssid, rawSsid.c_str(), sizeof(wifiResults[i].ssid) - 1);
    wifiResults[i].ssid[sizeof(wifiResults[i].ssid) - 1] = '\0';
    wifiResults[i].rssi = WiFi.RSSI(i);
    wifiResults[i].encryptionType = WiFi.encryptionType(i);
  }
  wifiCount = count;

  if (wifiCount > 0) {
    int maxRssi = wifiResults[0].rssi;
    if (maxRssi >= -55) {
      setRGBColor(0, 255, 0);
    } else if (maxRssi >= -75) {
      setRGBColor(255, 165, 0);
    } else {
      setRGBColor(255, 0, 0);
    }
  } else {
    setRGBColor(255, 0, 0);
  }

  wifiScanRunning = false;
  vTaskDelete(NULL);
}

void triggerWifiScan() {
  if (!wifiScanRunning) {
    wifiScanRunning = true;
    setRGBColor(0, 0, 150);
    xTaskCreatePinnedToCore(wifiScanTask, "WifiScanTask", 4096, NULL, 1, NULL, 0);
  }
}

void bleScanTask(void *pvParameters) {
  static bool bleInitialized = false;
  if (!bleInitialized) {
    BLEDevice::init("AashishOS");
    bleInitialized = true;
  }

  bleCount = 0;
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  pBLEScan->start(3, false);
  pBLEScan->clearResults();

  if (bleCount > 0) {
    setRGBColor(0, 255, 0);
  } else {
    setRGBColor(255, 0, 0);
  }

  bleScanRunning = false;
  vTaskDelete(NULL);
}

void triggerBleScan() {
  if (!bleScanRunning) {
    bleScanRunning = true;
    setRGBColor(0, 0, 150);
    xTaskCreatePinnedToCore(bleScanTask, "BleScanTask", 4096, NULL, 1, NULL, 0);
  }
}

void renderWifiList(Adafruit_SH1106G &display, int selectedIdx) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("WiFi Networks (");
  display.print(wifiCount);
  display.println(")");
  display.drawFastHLine(0, 9, 128, SH110X_WHITE);

  if (wifiScanRunning) {
    display.setCursor(0, 24);
    display.println("Scanning Core 0...");
    static int barPos = 0;
    barPos = (barPos + 4) % 110;
    display.drawRect(8, 42, 112, 10, SH110X_WHITE);
    display.fillRect(10 + barPos % 96, 44, 12, 6, SH110X_WHITE);
    display.display();
    return;
  }

  if (wifiCount == 0) {
    display.setCursor(0, 24);
    display.println("No networks. Click");
    display.println("joystick to scan.");
    display.display();
    return;
  }

  int startIdx = max(0, selectedIdx - 2);
  int endIdx = (wifiCount < startIdx + 4) ? (int)wifiCount : (startIdx + 4);

  int y = 12;
  for (int i = startIdx; i < endIdx; i++) {
    if (i == selectedIdx) {
      display.fillRect(0, y, 128, 11, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    } else {
      display.setTextColor(SH110X_WHITE);
    }
    
    display.setCursor(2, y + 2);
    char displaySsid[16];
    strncpy(displaySsid, wifiResults[i].ssid, 13);
    displaySsid[13] = '\0';
    if (strlen(wifiResults[i].ssid) > 13) strcat(displaySsid, "..");
    display.print(displaySsid);
    
    display.setCursor(95, y + 2);
    display.print(wifiResults[i].rssi);
    display.print("dB");
    
    y += 12;
  }
  display.display();
}

void renderBleList(Adafruit_SH1106G &display, int selectedIdx) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("BLE Devices (");
  display.print(bleCount);
  display.println(")");
  display.drawFastHLine(0, 9, 128, SH110X_WHITE);

  if (bleScanRunning) {
    display.setCursor(0, 24);
    display.println("Scanning Core 0...");
    static int barPos = 0;
    barPos = (barPos + 4) % 110;
    display.drawRect(8, 42, 112, 10, SH110X_WHITE);
    display.fillRect(10 + barPos % 96, 44, 12, 6, SH110X_WHITE);
    display.display();
    return;
  }

  if (bleCount == 0) {
    display.setCursor(0, 24);
    display.println("No devices. Click");
    display.println("joystick to scan.");
    display.display();
    return;
  }

  int startIdx = max(0, selectedIdx - 2);
  int endIdx = (bleCount < startIdx + 4) ? (int)bleCount : (startIdx + 4);

  int y = 12;
  for (int i = startIdx; i < endIdx; i++) {
    if (i == selectedIdx) {
      display.fillRect(0, y, 128, 11, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    } else {
      display.setTextColor(SH110X_WHITE);
    }
    
    display.setCursor(2, y + 2);
    char displayName[16];
    const char* targetName = strcmp(bleResults[i].name, "Unknown") == 0 ? bleResults[i].address : bleResults[i].name;
    strncpy(displayName, targetName, 13);
    displayName[13] = '\0';
    if (strlen(targetName) > 13) strcat(displayName, "..");
    display.print(displayName);
    
    display.setCursor(95, y + 2);
    display.print(bleResults[i].rssi);
    display.print("dB");
    
    y += 12;
  }
  display.display();
}
