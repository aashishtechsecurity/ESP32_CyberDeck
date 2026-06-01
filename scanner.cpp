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
portMUX_TYPE scannerMux = portMUX_INITIALIZER_UNLOCKED;
unsigned long wifiScanStartedAtMs = 0;
unsigned long bleScanStartedAtMs = 0;
const unsigned long SCAN_TIMEOUT_MS = 15000;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      String nameStorage = advertisedDevice.haveName() ? String(advertisedDevice.getName().c_str()) : String("Unknown");
      String addrStorage = String(advertisedDevice.getAddress().toString().c_str());
      const char* nameStr = nameStorage.c_str();
      const char* addrStr = addrStorage.c_str();
      int rssi = advertisedDevice.getRSSI();

      portENTER_CRITICAL(&scannerMux);
      if (bleCount < MAX_BLE_DEVICES) {
        bool exists = false;
        for (int i = 0; i < bleCount; i++) {
          if (strcmp(bleResults[i].address, addrStr) == 0) {
            exists = true;
            break;
          }
        }

        if (!exists) {
          int idx = bleCount;
          strncpy(bleResults[idx].name, nameStr, sizeof(bleResults[idx].name) - 1);
          bleResults[idx].name[sizeof(bleResults[idx].name) - 1] = '\0';

          strncpy(bleResults[idx].address, addrStr, sizeof(bleResults[idx].address) - 1);
          bleResults[idx].address[sizeof(bleResults[idx].address) - 1] = '\0';

          bleResults[idx].rssi = rssi;
          bleCount++;
        }
      }
      portEXIT_CRITICAL(&scannerMux);
    }
};

void wifiScanTask(void *pvParameters) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();
  int count = (n > MAX_NETWORKS) ? MAX_NETWORKS : n;
  portENTER_CRITICAL(&scannerMux);
  wifiCount = 0;
  portEXIT_CRITICAL(&scannerMux);

  for (int i = 0; i < count; i++) {
    String rawSsid = WiFi.SSID(i);
    portENTER_CRITICAL(&scannerMux);
    strncpy(wifiResults[i].ssid, rawSsid.c_str(), sizeof(wifiResults[i].ssid) - 1);
    wifiResults[i].ssid[sizeof(wifiResults[i].ssid) - 1] = '\0';
    wifiResults[i].rssi = WiFi.RSSI(i);
    wifiResults[i].encryptionType = WiFi.encryptionType(i);
    portEXIT_CRITICAL(&scannerMux);
  }
  portENTER_CRITICAL(&scannerMux);
  wifiCount = count;
  portEXIT_CRITICAL(&scannerMux);

  int localWifiCount;
  int maxRssi = -127;
  portENTER_CRITICAL(&scannerMux);
  localWifiCount = wifiCount;
  if (localWifiCount > 0) {
    maxRssi = wifiResults[0].rssi;
  }
  portEXIT_CRITICAL(&scannerMux);

  if (localWifiCount > 0) {
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

  portENTER_CRITICAL(&scannerMux);
  wifiScanRunning = false;
  wifiScanStartedAtMs = 0;
  portEXIT_CRITICAL(&scannerMux);
  vTaskDelete(NULL);
}

void triggerWifiScan() {
  bool canStart = false;
  portENTER_CRITICAL(&scannerMux);
  if (!wifiScanRunning) {
    wifiScanRunning = true;
    wifiScanStartedAtMs = millis();
    canStart = true;
  }
  portEXIT_CRITICAL(&scannerMux);
  if (canStart) {
    setRGBColor(0, 0, 150);
    BaseType_t rc = xTaskCreatePinnedToCore(wifiScanTask, "WifiScanTask", 4096, NULL, 1, NULL, 0);
    if (rc != pdPASS) {
      portENTER_CRITICAL(&scannerMux);
      wifiScanRunning = false;
      wifiScanStartedAtMs = 0;
      portEXIT_CRITICAL(&scannerMux);
      setRGBColor(255, 0, 0);
    }
  }
}

void bleScanTask(void *pvParameters) {
  static bool bleInitialized = false;
  if (!bleInitialized) {
    BLEDevice::init("AashishOS");
    bleInitialized = true;
  }

  portENTER_CRITICAL(&scannerMux);
  bleCount = 0;
  portEXIT_CRITICAL(&scannerMux);
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  pBLEScan->start(3, false);
  pBLEScan->clearResults();

  int localBleCount;
  portENTER_CRITICAL(&scannerMux);
  localBleCount = bleCount;
  portEXIT_CRITICAL(&scannerMux);
  if (localBleCount > 0) {
    setRGBColor(0, 255, 0);
  } else {
    setRGBColor(255, 0, 0);
  }

  portENTER_CRITICAL(&scannerMux);
  bleScanRunning = false;
  bleScanStartedAtMs = 0;
  portEXIT_CRITICAL(&scannerMux);
  vTaskDelete(NULL);
}

void triggerBleScan() {
  bool canStart = false;
  portENTER_CRITICAL(&scannerMux);
  if (!bleScanRunning) {
    bleScanRunning = true;
    bleScanStartedAtMs = millis();
    canStart = true;
  }
  portEXIT_CRITICAL(&scannerMux);
  if (canStart) {
    setRGBColor(0, 0, 150);
    BaseType_t rc = xTaskCreatePinnedToCore(bleScanTask, "BleScanTask", 4096, NULL, 1, NULL, 0);
    if (rc != pdPASS) {
      portENTER_CRITICAL(&scannerMux);
      bleScanRunning = false;
      bleScanStartedAtMs = 0;
      portEXIT_CRITICAL(&scannerMux);
      setRGBColor(255, 0, 0);
    }
  }
}

void pollScannerTimeouts() {
  unsigned long now = millis();
  bool wifiTimedOut = false;
  bool bleTimedOut = false;

  portENTER_CRITICAL(&scannerMux);
  if (wifiScanRunning && wifiScanStartedAtMs > 0 && (now - wifiScanStartedAtMs > SCAN_TIMEOUT_MS)) {
    wifiScanRunning = false;
    wifiScanStartedAtMs = 0;
    wifiTimedOut = true;
  }

  if (bleScanRunning && bleScanStartedAtMs > 0 && (now - bleScanStartedAtMs > SCAN_TIMEOUT_MS)) {
    bleScanRunning = false;
    bleScanStartedAtMs = 0;
    bleTimedOut = true;
  }
  portEXIT_CRITICAL(&scannerMux);

  if (wifiTimedOut || bleTimedOut) {
    setRGBColor(255, 0, 0);
  }
}

void renderWifiList(Adafruit_SH1106G &display, int selectedIdx) {
  WifiNetworkInfo wifiSnapshot[MAX_NETWORKS];
  int localWifiCount = 0;
  bool localWifiScanRunning = false;
  portENTER_CRITICAL(&scannerMux);
  localWifiCount = wifiCount;
  localWifiScanRunning = wifiScanRunning;
  for (int i = 0; i < localWifiCount && i < MAX_NETWORKS; i++) {
    wifiSnapshot[i] = wifiResults[i];
  }
  portEXIT_CRITICAL(&scannerMux);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("WiFi Networks (");
  display.print(localWifiCount);
  display.println(")");
  display.drawFastHLine(0, 9, 128, SH110X_WHITE);

  if (localWifiScanRunning) {
    display.setCursor(0, 24);
    display.println("Scanning Core 0...");
    static int barPos = 0;
    barPos = (barPos + 4) % 110;
    display.drawRect(8, 42, 112, 10, SH110X_WHITE);
    display.fillRect(10 + barPos % 96, 44, 12, 6, SH110X_WHITE);
    display.display();
    return;
  }

  if (localWifiCount == 0) {
    display.setCursor(0, 24);
    display.println("No networks. Click");
    display.println("joystick to scan.");
    display.display();
    return;
  }

  int startIdx = max(0, selectedIdx - 2);
  int endIdx = (localWifiCount < startIdx + 4) ? localWifiCount : (startIdx + 4);

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
    strncpy(displaySsid, wifiSnapshot[i].ssid, 13);
    displaySsid[13] = '\0';
    if (strlen(wifiSnapshot[i].ssid) > 13) strcat(displaySsid, "..");
    display.print(displaySsid);
    
    display.setCursor(95, y + 2);
    display.print(wifiSnapshot[i].rssi);
    display.print("dB");
    
    y += 12;
  }
  display.display();
}

void renderBleList(Adafruit_SH1106G &display, int selectedIdx) {
  BleDeviceInfo bleSnapshot[MAX_BLE_DEVICES];
  int localBleCount = 0;
  bool localBleScanRunning = false;
  portENTER_CRITICAL(&scannerMux);
  localBleCount = bleCount;
  localBleScanRunning = bleScanRunning;
  for (int i = 0; i < localBleCount && i < MAX_BLE_DEVICES; i++) {
    bleSnapshot[i] = bleResults[i];
  }
  portEXIT_CRITICAL(&scannerMux);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("BLE Devices (");
  display.print(localBleCount);
  display.println(")");
  display.drawFastHLine(0, 9, 128, SH110X_WHITE);

  if (localBleScanRunning) {
    display.setCursor(0, 24);
    display.println("Scanning Core 0...");
    static int barPos = 0;
    barPos = (barPos + 4) % 110;
    display.drawRect(8, 42, 112, 10, SH110X_WHITE);
    display.fillRect(10 + barPos % 96, 44, 12, 6, SH110X_WHITE);
    display.display();
    return;
  }

  if (localBleCount == 0) {
    display.setCursor(0, 24);
    display.println("No devices. Click");
    display.println("joystick to scan.");
    display.display();
    return;
  }

  int startIdx = max(0, selectedIdx - 2);
  int endIdx = (localBleCount < startIdx + 4) ? localBleCount : (startIdx + 4);

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
    const char* targetName = strcmp(bleSnapshot[i].name, "Unknown") == 0 ? bleSnapshot[i].address : bleSnapshot[i].name;
    strncpy(displayName, targetName, 13);
    displayName[13] = '\0';
    if (strlen(targetName) > 13) strcat(displayName, "..");
    display.print(displayName);
    
    display.setCursor(95, y + 2);
    display.print(bleSnapshot[i].rssi);
    display.print("dB");
    
    y += 12;
  }
  display.display();
}
