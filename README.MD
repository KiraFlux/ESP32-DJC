# ESP32 Dual Joystick Controller (DJC)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

An open-source remote controller based on ESP32 with dual analog joysticks, designed for DIY robotics projects.

<img src="./Images/2026.04.10.jpg" style="width: auto; height: 600px">

## Hardware

| Component     | Specification                                             | Notes                               |
| ------------- | --------------------------------------------------------- | ----------------------------------- |
| MCU           | ESP32 (any DevKit variant)                                | Can be used with an expansion board |
| Communication | ESPNOW                                                    | Direct ESP32-to-ESP32               |
| Joysticks     | 2x HW-504                                                 |                                     |
| Buttons       | 2x external micro switch (NO)                             | e.g., V-153-1C25                    |
| Display       | ST7735 (160x128, SPI)                                     | SSD1306 support planned             |
| Power Supply  | 1S1P 18650, step-up Voltage Converter module XR2981 to 5V | todo: add TP4056                    |

## Pin Configuration

| Name              | Component             | №    | Pin   | Board Network    |
| ----------------- | --------------------- | ---- | ----- | ---------------- |
| Joystick Left     | HW-504                | 1    | GND   | Devboard-GND     |
|                   |                       | 2    | +5V   | Devboard-3V3     |
|                   |                       | 3    | VRX   | Devboard-GPIO_32 |
|                   |                       | 4    | VRY   | Devboard-GPIO_33 |
|                   |                       | 5    | SW    | *NC*             |
|                   |                       |      |       |                  |
| Joystick Right    | HW-504                | 1    | GND   | Devboard-GND     |
|                   |                       | 2    | +5V   | Devboard-3V3     |
|                   |                       | 3    | VRX   | Devboard-GPIO_35 |
|                   |                       | 4    | VRY   | Devboard-GPIO_34 |
|                   |                       | 5    | SW    | *NC*             |
|                   |                       |      |       |                  |
| Button Left       | V-153-1C25            | 1    | COM   | Devboard-GND     |
|                   |                       | 2    | NO    | Devboard-GPIO_14 |
|                   |                       | 3    | NC    | *NC*             |
|                   |                       |      |       |                  |
| Button Right      | V-153-1C25            | 1    | COM   | Devboard-GND     |
|                   |                       | 2    | NO    | Devboard-GPIO_21 |
|                   |                       | 3    | NC    | *NC*             |
|                   |                       |      |       |                  |
| Display           | ST7735                | 1    | LED   | Devboard-5V      |
|                   | (Module SPI 160x128)  | 2    | SCK   | Devboard-GPIO_18 |
|                   |                       | 3    | MOSI  | Devboard-GPIO_23 |
|                   |                       | 4    | DC    | Devboard-GPIO_2  |
|                   |                       | 5    | Reset | Devboard-GPIO_15 |
|                   |                       | 6    | CS    | Devboard-GPIO_5  |
|                   |                       | 7    | GND   | Devboard-GND     |
|                   |                       | 8    | VCC   | Devboard-3V3     |
|                   |                       |      |       |                  |
| Power Switch      | KCD-11-2P             | 1    | COM   | B+               |
|                   |                       | 2    | NO    | PowerSwitch-NO   |
|                   |                       |      |       |                  |
| Voltage Converter | XR2981                | X1-1 | OUT+  | Devboard-5V      |
|                   | (Step-up Module)      | X1-2 | OUT-  | Devboard-GND     |
|                   |                       | X2-1 | IN+   | PowerSwitch-NO   |
|                   |                       | X2-2 | IN-   | B-               |
|                   |                       |      |       |                  |
| Battery Sense     | Resistor R1 (100 kΩ)  | 1    | –     | PowerSwitch‑NO   |
|                   |                       | 2    | –     | Devboard‑GPIO_36 |
|                   | Resistor R2 (33 kΩ)   | 1    | –     | Devboard‑GPIO_36 |
|                   |                       | 2    | –     | Devboard‑GND     |
|                   | Capacitor C1 (0.1 µF) | 1    | –     | Devboard‑GPIO_36 |
|                   |                       | 2    | –     | Devboard‑GND     |

<blockquote>    

### NOTE:

*NC* means *Not Connected*

Devboard means ESP32 Devkit v4 prototyping board

ST7735 backlight LED **requires** a current‑limiting resistor (440Ω, e.g., two 220Ω in series) when powered from 5V.

Buttons are external micro switches `NO` (normally open) connected between GND and the respective GPIO. Internal pull‑ups are enabled in firmware.

B+ and B- denote battery contacts

</blockquote>

## ESP32 Pin Status

| Left Header | Status | Project Assignment      | Right Header | Status | Project Assignment             |
| ----------: | :----: | :---------------------- | -----------: | :----: | :----------------------------- |
|     **3V3** |   ✓    | Logic power             |      **GND** |   ✓    | Common ground                  |
|      **EN** |   ✓    | Reset (EN button)       |   **GPIO23** |   ✓    | SPI MOSI (display + SD)        |
| **GPIO 36** |   ✓    | Battery voltage divider |  **GPIO 22** |   ○    | *Free*                         |
| **GPIO 39** |   ○    | *Free (input only)*     |   **GPIO 1** |   ✓    | UART0 TX (Serial debug)        |
| **GPIO 34** |   ✓    | Right joystick Y        |   **GPIO 3** |   ✓    | UART0 RX (Serial debug)        |
| **GPIO 35** |   ✓    | Right joystick X        |  **GPIO 21** |   ✓    | Right button                   |
| **GPIO 32** |   ✓    | Left joystick X         |  **GPIO 19** |   ✓    | SPI MISO (SD card)             |
| **GPIO 33** |   ✓    | Left joystick Y         |  **GPIO 18** |   ✓    | SPI SCK (display + SD)         |
| **GPIO 25** |   ○    | *Free*                  |   **GPIO 5** |   ✓    | SPI CS (display)               |
| **GPIO 26** |   ○    | *Free (DAC)*            |  **GPIO 17** |   ✓    | SPI CS (SD card)               |
| **GPIO 27** |   ○    | *Free*                  |  **GPIO 16** |   ○    | *Free*                         |
| **GPIO 14** |   ✓    | Left button             |   **GPIO 4** |   ○    | *Free*                         |
| **GPIO 12** |   ✗    | *Avoid (MTDI)*          |   **GPIO 0** |   ✓    | BOOT button (use with caution) |
|     **GND** |   ✓    | Common ground           |   **GPIO 2** |   ✓    | Display DC                     |
|     **VIN** |   ✓    | 5V input (from XR2981)  |  **GPIO 15** |   ✓    | Display RESET                  |

<blockquote>

### Legend

| Symbol | Meaning         | Note                                         |
| :----: | --------------- | :------------------------------------------- |
|   ✓    | **Occupied**    | Already used in the project or reserved      |
|   ○    | **Free**        | Available for new components                 |
|   ✗    | **Unavailable** | **Not** recommended for use (boot conflicts) |

### Notes

| Feature | Used pins             | Note                                                                   |
| ------: | :-------------------- | ---------------------------------------------------------------------- |
|    MTDI | **GPIO 12**           | controls flash voltage at startup; using it may cause boot instability |
|   UART0 | **GPIO 1 / GPIO 3**   | Serial                                                                 |
|    BOOT | **GPIO 0**            | It can be used as GPIO if held high at boot.                           |
|         | **GPIO 39**           | input-only and lacks an internal pull‑up resistor                      |
|     DAC | **GPIO 25 / GPIO 26** | Can be used as DAC outputs                                             |

</blockquote>

## Software

Firmware is a **PlatformIO** project located in the [`DJC-Firmware`](./DJC-Firmware) directory. All dependencies are declared and managed in [`platformio.ini`](./DJC-Firmware/platformio.ini).

The project uses:
- [KiraFlux‑Toolkit](https://github.com/KiraFlux/KiraFlux-Toolkit.git)
- [MAVLink](https://github.com/okalachev/MAVLink.git)

### Build

A [`makefile`](./makefile) wraps common PlatformIO commands for convenience.

| Command      | Action                                                      |
| ------------ | ----------------------------------------------------------- |
| `make`       | Compile the project (`pio run`)                             |
| `make u`     | Compile and upload to the ESP32 (`pio run --target upload`) |
| `make m`     | Open serial monitor (`pio device monitor`)                  |
| `make clean` | Delete compiled objects (`pio run --target clean`)          |

### Features

| Feature                                 | Status                                                             |
| --------------------------------------- | ------------------------------------------------------------------ |
| Dual joystick with calibration          | Implemented                                                        |
| ST7735 display (SPI)                    | Implemented                                                        |
| ESPNOW peer discovery & connection      | Implemented                                                        |
| Raw and MAVLink control modes           | Implemented (Basic)                                                |
| Persistent configuration (NVS)          | Implemented                                                        |
| MAVLink telemetry (partial)             | SCALED_IMU, ATTITUDE_QUATERNION, SERIAL_CONTROL messages supported |
| Peer explorer with signal age           | Implemented (shows last seen time)                                 |
| On‑screen text input (virtual keyboard) | Implemented (Basic)                                                |

### Planned

- Finalize power supply documentation (battery, boost converter, charging)
- Battery-powered operation with 18650 1S1P Li‑ion cell + boost converter (5V) for devkit board
- Create schematic in KiCad for the DIY shield
- DIY shield for devkit board (single‑sided PCB, toner transfer method) with integrated power circuitry
- Idle sleep for power saving
- Calibration visual feedback
- Confirmation for reset / delete peer
- Custom peer names/aliases
- Extended MAVLink telemetry (position, battery, flight mode)
- CLI over Serial
- SSD1306 support via compile flag
- Hybrid storage (LittleFS + SD card)

### Known Limitations

- Only ST7735 display is currently supported
- MAVLink telemetry limited to basic IMU/attitude
- No CLI, no sleep

## Usage

The device operates in three modes. Switching between UI Navigation and Control is done with the left button; Virtual Keyboard Input appears automatically when a text field is activated.

| Mode                       | Purpose                                | Activation                                                          |
| -------------------------- | -------------------------------------- | ------------------------------------------------------------------- |
| **UI Navigation**          | Menu navigation and widget interaction | Default after power‑on; return from Control by pressing left button |
| **Control**                | Transmit joystick data over ESPNOW     | Left button from UI Navigation                                      |
| **Virtual Keyboard Input** | Text entry                             | Automatically when a text field is activated                        |

### Controls by Mode

| Control           | UI Navigation                                 | Control (Raw)               | Control (MAVLink)           | Virtual Keyboard Input               |
| ----------------- | --------------------------------------------- | --------------------------- | --------------------------- | ------------------------------------ |
| **Left stick X**  | *Unused*                                      | `left_x`                    | `Yaw`                       | *Unused*                             |
| **Left stick Y**  | *Unused*                                      | `left_y`                    | `Thrust`                    | *Unused*                             |
| **Right stick X** | Change value of active widget (left/right)    | `right_x`                   | `Roll`                      | Move cursor horizontally on keyboard |
| **Right stick Y** | Move cursor between widgets (up/down)         | `right_y`                   | `Pitch`                     | Move cursor vertically on keyboard   |
| **Left button**   | Switch to **Control**                         | Return to **UI Navigation** | Return to **UI Navigation** | **Close virtual keyboard**           |
| **Right button**  | Activate selected widget (press, follow link) | *Ignored*                   | *Ignored*                   | **Enter the selected key**           |

**Notes:**
- When the virtual keyboard is active, the left button closes it and does not switch modes.
- In Control mode, the right joystick does not affect the UI – all its movements are transmitted.

## Models

3D models (Kompas 3D format) are located in `Models-Legacy` (outdated) and `Models` (under development).  
No production‑ready files yet.

## Images

<blockquote>

## v0.1.x

| `v0.1.0` (SSD1306 display)                                              | `v0.1.1` (Using ST7735 display)                                         |
| ----------------------------------------------------------------------- | ----------------------------------------------------------------------- |
| <img src="./Images/2025.10.19.jpg" style="width: auto; height: 400px;"> | <img src="./Images/2026.01.23.jpg" style="width: auto; height: 400px;"> |

## v0.2.x

| `v0.2.0` (Add microswitch buttons)                                      | `v0.2.1` (Improved cable management)                                    |
| ----------------------------------------------------------------------- | ----------------------------------------------------------------------- |
| <img src="./Images/2026.04.09.jpg" style="width: auto; height: 400px;"> | <img src="./Images/2026.04.10.jpg" style="width: auto; height: 400px;"> |

</blockquote>

---

## License

This project is licensed under the **GNU General Public License v3.0 or later** (SPDX: `GPL-3.0-or-later`).

See the [LICENSE](LICENSE) file for the full text.