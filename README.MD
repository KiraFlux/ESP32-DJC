# ESP32 Dual Joystick Controller (DJC)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

An open-source remote controller based on ESP32 with dual analog joysticks, designed for DIY robotics projects.

<img src="./Images/2026.04.10.jpg" style="width: auto; height: 600px; max-width: 100%;">

## Hardware

| Component     | Specification                 | Notes                               |
| ------------- | ----------------------------- | ----------------------------------- |
| MCU           | ESP32 (any DevKit variant)    | Can be used with an expansion board |
| Joysticks     | 2x HW-504                     |                                     |
| Buttons       | 2x external micro switch (NO) | e.g., V-153-1C25                    |
| Display       | ST7735 (160x128, SPI)         | SSD1306 support planned             |
| Communication | ESPNOW                        | Direct ESP32-to-ESP32               |

## Pin Configuration

| Component  | Name             | №   | Pin   | ESP32-Pin |
| ---------- | ---------------- | --- | ----- | --------- |
| HW-504     | Joystick (Left)  | 1   | GND   | GND       |
|            |                  | 2   | +5V   | 3V3       |
|            |                  | 3   | VRX   | GPIO 32   |
|            |                  | 4   | VRY   | GPIO 33   |
|            |                  | 5   | SW    | *NC*      |
|            |                  |     |       |           |
| HW-504     | Joystick (Right) | 1   | GND   | GND       |
|            |                  | 2   | +5V   | 3V3       |
|            |                  | 3   | VRX   | GPIO 35   |
|            |                  | 4   | VRY   | GPIO 34   |
|            |                  | 5   | SW    | *NC*      |
|            |                  |     |       |           |
| V-153-1C25 | Button (Left)    | 1   | COM   | GND       |
|            |                  | 2   | NO    | GPIO 14   |
|            |                  | 3   | NC    | *NC*      |
|            |                  |     |       |           |
| V-153-1C25 | Button (Right)   | 1   | COM   | GND       |
|            |                  | 2   | NO    | GPIO 21   |
|            |                  | 3   | NC    | *NC*      |
|            |                  |     |       |           |
| ST7735     | Display          | 1   | LED   | 5V        |
|            | (160x128, SPI)   | 2   | SCK   | GPIO 18   |
|            |                  | 3   | MOSI  | GPIO 23   |
|            |                  | 4   | DC    | GPIO 2    |
|            |                  | 5   | Reset | GPIO 15   |
|            |                  | 6   | CS    | GPIO 5    |
|            |                  | 7   | GND   | GND       |
|            |                  | 8   | VCC   | 3V3       |

<blockquote>

### NOTE:

*NC* means *Not Connected*

ST7735 backlight LED **requires** a 440Ω (two 220Ω in series) current-limiting resistor to 5V.

Buttons are external micro switches `NO` (normally open) connected between GND and the respective GPIO. Internal pull‑ups are enabled in firmware.

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

| Feature                                 | Status                                                   |
| --------------------------------------- | -------------------------------------------------------- |
| Dual joystick with calibration          | Implemented                                              |
| ST7735 display (SPI)                    | Implemented                                              |
| ESPNOW peer discovery & connection      | Implemented                                              |
| Raw and MAVLink control modes           | Implemented (Basic)                                      |
| Persistent configuration (NVS)          | Implemented                                              |
| MAVLink telemetry (partial)             | SCALED_IMU, ATTITUDE_QUATERNION, SERIAL_CONTROL messages |
| Peer explorer with signal age           | Implemented                                              |
| On‑screen text input (virtual keyboard) | Implemented (Basic)                                      |

### Planned

- Power supply documentation (currently only USB, no shield)
- Battery-powered operation with 18650 1S1P Li‑ion cell + boost converter (5V) for devkit board
- Create schematic in KiCad for the DIY shield
- DIY shield for devkit board (single‑sided PCB, toner transfer method) with integrated power circuitry
- Idle sleep for power saving
- Calibration visual feedback
- Active peer indicator on all pages
- Confirmation for reset / delete peer
- Peer nicknames
- Extended MAVLink telemetry (position, battery, flight mode)
- CLI over Serial
- SSD1306 support via compile flag

### Known Limitations

- Only ST7735 display is currently supported
- MAVLink telemetry limited to basic IMU/attitude
- No CLI, no sleep

## Usage

The device operates in three modes. Switching between UiNavigation and Control is done with the left button; VirtualKeyboardInput appears automatically when a text field is activated.

| Mode                     | Purpose                                | Activation                                                          |
| ------------------------ | -------------------------------------- | ------------------------------------------------------------------- |
| **UiNavigation**         | Menu navigation and widget interaction | Default after power‑on; return from Control by pressing left button |
| **Control**              | Transmit joystick data over ESPNOW     | Left button from UiNavigation                                       |
| **VirtualKeyboardInput** | Text entry (e.g., device name)         | Automatically when a text field is activated                        |

### Controls by Mode

| Control           | UiNavigation                                  | Control (Raw)              | Control (MAVLink)          | VirtualKeyboardInput                 |
| ----------------- | --------------------------------------------- | -------------------------- | -------------------------- | ------------------------------------ |
| **Left stick X**  | *Unused*                                      | `left_x`                   | `Yaw`                      | *Unused*                             |
| **Left stick Y**  | *Unused*                                      | `left_y`                   | `Thrust`                   | *Unused*                             |
| **Right stick X** | Change value of active widget (left/right)    | `right_x`                  | `Roll`                     | Move cursor horizontally on keyboard |
| **Right stick Y** | Move cursor between widgets (up/down)         | `right_y`                  | `Pitch`                    | Move cursor vertically on keyboard   |
| **Left button**   | Switch to **Control**                         | Return to **UiNavigation** | Return to **UiNavigation** | **Close virtual keyboard**           |
| **Right button**  | Activate selected widget (press, follow link) | *Ignored*                  | *Ignored*                  | **Enter the selected key**           |

**Notes:**
- When the virtual keyboard is active, the left button closes it and does not switch modes.
- In Control mode, the right joystick does not affect the UI – all its movements are transmitted.
- The active peer (MAC address) is currently only shown on the `PeerExplorerPage`.

## Models

3D models (Kompas 3D format) are in `Models-Legacy` (outdated) and `Models` (under development).  
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