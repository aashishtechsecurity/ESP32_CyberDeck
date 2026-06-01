# Aashish OS - ESP32 CyberDeck Mini

Aashish OS is a multi-page handheld UI system for ESP32 with a 1.3-inch SH1106 OLED, 5-axis analog joystick, and RGB status LED.

---

## Hardware Setup and Pinout

| ESP32 Pin | Connected To          | Description |
| --------- | --------------------- | ----------- |
| **GPIO2**  | RGB Red               | Status indicator red pin |
| **GPIO4**  | RGB Blue              | Status indicator blue pin |
| **GPIO15** | RGB Green             | Status indicator green pin |
| **GPIO21** | OLED SDA              | I2C data |
| **GPIO22** | OLED SCL              | I2C clock |
| **GPIO25** | Joystick Button       | Center click (`INPUT_PULLUP`) |
| **GPIO34** | Joystick X-Axis       | Horizontal analog |
| **GPIO35** | Joystick Y-Axis       | Vertical analog |
| **3.3V**   | OLED + Joystick VCC   | Power |
| **GND**    | OLED + RGB + Joystick | Common ground |

---

## Key Features

- Cyberpunk-style animated boot sequence
- App launcher carousel with joystick navigation
- FreeRTOS Wi-Fi and BLE scanning tasks
- Idle animations/screensaver with quick wake
- Light sleep after long inactivity
- Startup joystick center auto-calibration

---

## Project Structure

- `ESP32_CyberDeck.ino`: Main app state machine and render loop
- `config.h`: Pin mapping and UI constants
- `hardware.*`: Joystick, button interrupt, RGB hardware I/O
- `matrix_boot.*`: Boot animation/rendering
- `scanner.*`: Wi-Fi/BLE scanning + list UI
- `dashboard.*`: System monitor UI
- `games.*`: Snake + Flappy game logic/rendering
- `rgb_ctrl.*`: RGB effect modes
- `animations.*`: Screensaver scene animations

---

## Build and Upload (Arduino IDE)

1. Open **Arduino IDE**.
2. Install board support:
   - **Tools -> Board -> Boards Manager...**
   - Install **ESP32 by Espressif Systems**.
3. Install libraries from **Library Manager**:
   - **Adafruit GFX Library**
   - **Adafruit SH110X**
4. Select board:
   - **Tools -> Board -> ESP32 Dev Module**
5. Select COM port and click **Upload**.

---

## Build and Upload (Arduino CLI)

Example commands:

```bash
arduino-cli core update-index
arduino-cli core install esp32:esp32
arduino-cli lib install "Adafruit GFX Library"
arduino-cli lib install "Adafruit SH110X"
arduino-cli compile --fqbn esp32:esp32:esp32 ESP32_CyberDeck.ino
arduino-cli upload -p <COM_PORT> --fqbn esp32:esp32:esp32 ESP32_CyberDeck.ino
```

Replace `<COM_PORT>` with your device port, for example `COM5`.

---

## Build and Upload (PlatformIO)

`platformio.ini` is included for reproducible builds.

```bash
pio run
pio run -t upload --upload-port <COM_PORT>
pio device monitor -b 115200
```

Replace `<COM_PORT>` with your device port, for example `COM5`.
