#ifndef SCANNER_H
#define SCANNER_H

#include <Adafruit_SH110X.h>

struct WifiNetworkInfo {
  char ssid[32];
  int32_t rssi;
  uint8_t encryptionType;
};

struct BleDeviceInfo {
  char name[32];
  char address[20];
  int rssi;
};

#define MAX_NETWORKS 15
#define MAX_BLE_DEVICES 15

// External declarations of shared variables
extern WifiNetworkInfo wifiResults[MAX_NETWORKS];
extern volatile int wifiCount;
extern volatile bool wifiScanRunning;

extern BleDeviceInfo bleResults[MAX_BLE_DEVICES];
extern volatile int bleCount;
extern volatile bool bleScanRunning;

// Scanner API Declarations
void triggerWifiScan();
void triggerBleScan();
void pollScannerTimeouts();
void renderWifiList(Adafruit_SH1106G &display, int selectedIdx);
void renderBleList(Adafruit_SH1106G &display, int selectedIdx);

#endif // SCANNER_H
