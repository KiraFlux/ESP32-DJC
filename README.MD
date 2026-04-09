# ESP32 Dual Joystick Controller (DJC)

An open-source remote controller based on ESP32 with dual analog joysticks, designed for DIY robotics projects.

![2026.04.09.jpg](./Images/2026.04.09.jpg)

## Hardware

| Component     | Specification                 | Notes                   |
| ------------- | ----------------------------- | ----------------------- |
| MCU           | ESP32 (any DevKit variant)    |                         |
| Joysticks     | 2x HW-504                     |                         |
| Buttons       | 2x external micro switch (NO) | e.g., V-153-1C25        |
| Display       | ST7735 (160x128, SPI)         | SSD1306 support planned |
| Communication | ESPNOW                        | Direct ESP32-to-ESP32   |

## Pin Configuration

| Component    | Name             | N   | Pin   | ESP32-Pin |
| ------------ | ---------------- | --- | ----- | --------- |
| HW-504       | Joystick (Left)  | 1   | GND   | GND       |
|              |                  | 2   | +5V   | 3V3       |
|              |                  | 3   | VRX   | GPIO 32   |
|              |                  | 4   | VRY   | GPIO 33   |
|              |                  |     |       |           |
| HW-504       | Joystick (Right) | 1   | GND   | GND       |
|              |                  | 2   | +5V   | 3V3       |
|              |                  | 3   | VRX   | GPIO 35   |
|              |                  | 4   | VRY   | GPIO 34   |
|              |                  |     |       |           |
| Micro Switch | Button (Left)    | 1   | COM   | GND       |
| (V-153-1C25) |                  | 2   | NO    | GPIO 14   |
|              |                  |     |       |           |
| Micro Switch | Button (Right)   | 1   | COM   | GND       |
| (V-153-1C25) |                  | 2   | NO    | GPIO 21   |
|              |                  |     |       |           |
| ST7735       | Display          | 1   | GND   | GND       |
|              | (160x128, SPI)   | 2   | Reset | GPIO 15   |
|              |                  | 3   | VCC   | 3V3       |
|              |                  | 4   | DC    | GPIO 2    |
|              |                  | 5   | CS    | GPIO 5    |
|              |                  | 6   | SCK   | GPIO 18   |
|              |                  | 7   | MOSI  | GPIO 23   |
|              |                  | 8   | LED   | 5V        |

<blockquote>

### NOTE:

ST7735 backlight LED requires a (220 + 220)Ω current-limiting resistors to 5V.

Buttons are external micro switches NO (normally open) connected between GND and the respective GPIO. Internal pull‑ups are enabled in firmware.

</blockquote>

## Software

Firmware is a **PlatformIO** project located in the [`DJC-Firmware/`](./DJC-Firmware/) directory. All dependencies are declared and managed in [`platformio.ini`](./DJC-Firmware/platformio.ini).

The project uses:
- [KiraFlux‑Toolkit](https://github.com/KiraFlux/KiraFlux-Toolkit.git)
- [MAVLink](https://github.com/okalachev/MAVLink.git)

### Build

A [`makefile`](./DJC-Firmware/makefile) wraps common PlatformIO commands for convenience.

| Command      | Action                                                      |
| ------------ | ----------------------------------------------------------- |
| `make`       | Compile the project (`pio run`)                             |
| `make u`     | Compile and upload to the ESP32 (`pio run --target upload`) |
| `make m`     | Open serial monitor (`pio device monitor`)                  |
| `make clean` | Delete compiled objects (`pio run --target clean`)          |

### Features

| Feature                                 | Status                          |
| --------------------------------------- | ------------------------------- |
| Dual joystick with calibration          | Implemented                     |
| ST7735 display (SPI)                    | Implemented                     |
| ESPNOW peer discovery & connection      | Implemented                     |
| Raw and MAVLink control modes           | Implemented (Basic)             |
| Persistent configuration (NVS)          | Implemented                     |
| MAVLink telemetry (partial)             | SCALED_IMU, ATTITUDE_QUATERNION |
| Peer explorer with signal age           | Implemented                     |
| On‑screen text input (virtual keyboard) | Implementation (Basic)          |

### Planned

- Active peer indicator on all pages
- Idle sleep for power saving
- Confirmation for reset / delete peer
- Calibration visual feedback
- Extended MAVLink telemetry (position, battery, flight mode)
- CLI over Serial
- Peer nicknames
- Auto‑add devices from heartbeat
- SSD1306 support via compile flag

### Known Limitations

- Only ST7735 display supported currently
- MAVLink telemetry limited to basic IMU/attitude
- No CLI, no sleep

## Usage

The device operates in three modes, switching automatically or via the left button.

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

3D models (Kompas 3D format) are in `Models-Legacy/` (outdated) and `Models/` (under development).  
No production‑ready STL files yet.

## Images

| Original `v0.1.0` (SSD1306 display)        | Early `v0.1.1` (display ST7735)            | Current `v0.2.0` (Add microswitch buttons) |
| ------------------------------------------ | ------------------------------------------ | ------------------------------------------ |
| ![2025.10.19.jpg](./Images/2025.10.19.jpg) | ![2026.01.23.jpg](./Images/2026.01.23.jpg) | ![2026.04.09.jpg](./Images/2026.04.09.jpg) |

## License

MIT License. See [LICENSE](./LICENSE) file for details.