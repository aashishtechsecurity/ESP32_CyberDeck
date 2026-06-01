# Aashish OS - ESP32 CyberDeck Mini

Aashish OS is a high-performance, multi-page handheld operating system designed for the ESP32 paired with a 1.3-inch SH1106 OLED screen, a 5-axis analog joystick, and an RGB status LED.

---

## 🛠️ Hardware Setup & Pinout

| ESP32 Pin | Connected To          | Description |
| --------- | --------------------- | ----------- |
| **GPIO2**  | RGB Red               | Status indicator Red pin |
| **GPIO4**  | RGB Blue              | Status indicator Blue pin |
| **GPIO15** | RGB Green             | Status indicator Green pin |
| **GPIO21** | OLED SDA              | Hardware I2C Data line |
| **GPIO22** | OLED SCL              | Hardware I2C Clock line |
| **GPIO25** | Joystick Button       | Central push click (PULLUP) |
| **GPIO34** | Joystick X-Axis       | Horizontal movement analog pin |
| **GPIO35** | Joystick Y-Axis       | Vertical movement analog pin |
| **3.3V**   | OLED + Joystick VCC   | Power supply line |
| **GND**    | OLED + RGB + Joystick | Shared Ground line |

---

## 🚀 Key Features

* **Cyberpunk Glitch Boot sequence**: Start up with dynamic Matrix rain, a hexadecimal sector decrypting log, sci-fi crosshair frames, binary character decrypting animations, and synced RGB transitions.
* **Cover-Page Launcher Carousel**: Navigate between 8 different apps using Joystick Left/Right. Launch applications with a center Click.
* **FreeRTOS Async Network Scanners**: Offload WiFi and BLE scans to ESP32 Core 0, keeping the UI rendering thread responsive with active loading indicators.
* **Physics-based Idle Screensaver**: Enter a screensaver showing interactive Bike Riding and Girl & Flower animations after 5 seconds of inactivity. Wake up instantly upon joystick movement.
* **Low Power Sleep**: Autonomously enter ESP32 **Light Sleep** after 5 minutes of idle screensavers. Wake up by clicking the joystick center button.
* **Dynamic Auto-Calibration**: Sample joystick offsets at startup to eliminate hardware center drift.

---

## 📂 Project Architecture

* `ESP32_CyberDeck.ino`: Setup configurations, thread offloading wrappers, sleep state rules, and horizontal layout swappers.
* `config.h`: Central pins and analog calibration variables.
* `hardware.h`: Calibration filters, debounce engines, and RGB LED PWM controllers.
* `matrix_boot.h`: Dynamic boot loader animations and diagnostic panels.
* `scanner.h`: Background FreeRTOS scanner tasks and UI listing pages.
* `dashboard.h`: Cyber monitor UI with graphs and scroll diagnostics log.
* `games.h`: Responsive Snake game with a dynamic key buffering system.
* `rgb_ctrl.h`: Strobe, rainbow cycle, and breathing modes controller.
* `animations.h`: Custom physics engine rendering rotating wheel spokes, exhaust smoke, sways, and fluttering particles.

---

## 💾 Library Installation & Upload

1. Open **Arduino IDE**.
2. Go to **Sketch** -> **Include Library** -> **Manage Libraries...** and search for and install:
   * **Adafruit GFX Library**
   * **Adafruit SH110X**
3. Select **ESP32 Dev Module** under **Tools** -> **Board**.
4. Connect your ESP32, select its COM Port under **Tools** -> **Port**, and hit **Upload**!
