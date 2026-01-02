# ESP32 Dual Joystick Controller (DJC)

An open-source, customizable remote controller based on ESP32 with dual analog joysticks, an OLED display, and support for multiple communication protocols. Designed for DIY drone, rover, and robotics projects.

---

## Features

- Dual analog joysticks with push-buttons
- 128×64 OLED display (SSD1306, I2C)
- Multi-protocol communication:
  - ESPNOW – low-latency, direct ESP32-to-ESP32 control
  - MAVLink – compatibility with ArduPilot, PX4, and other flight controllers
- Modular firmware architecture – easy to extend and customize
- Built-in UI system with interactive menus and real-time data display
- 3D-printable case (included in the repository)

---

## Hardware Requirements

| Component | Specification | Notes |
|-----------|---------------|-------|
| MCU | ESP32 DevKit or similar | Any ESP32 with sufficient GPIOs |
| Joysticks | 2× analog (PS2-style) | With integrated push-button |
| Display | SSD1306, 128×64, I2C | |
| Power | 3.3V regulated supply | Can be powered via USB during development |

### Default Pin Mapping (configurable in `Periphery.hpp`)

| Function | GPIO |
|----------|------|
| Left joystick X | GPIO 32 |
| Left joystick Y | GPIO 33 |
| Left joystick button | GPIO 15 |
| Right joystick X | GPIO 35 |
| Right joystick Y | GPIO 34 |
| Right joystick button | GPIO 4 |
| Display SDA | GPIO 21 (default I2C) |
| Display SCL | GPIO 22 (default I2C) |

---

## Software Setup

### 1. Clone the repository with submodules
```bash
git clone --recursive https://github.com/KiraFlux/ESP32-DJC.git
cd ESP32-DJC
```

### 2. Open in PlatformIO
- Install [VS Code](https://code.visualstudio.com/) with the [PlatformIO extension](https://platformio.org/install/ide?install=vscode)
- Open the `DJC-Firmware` folder in VS Code
- PlatformIO will automatically install dependencies

### 3. Build and upload
- Connect your ESP32 via USB
- Click Build (✓) then Upload (→) in PlatformIO
- Monitor serial output at 115200 baud

---

## How It Works

### Firmware Architecture
The controller uses a behavior-based system. Each "behavior" represents a separate operational mode that can be switched at runtime:

- **SimpleControl** – Sends raw joystick values via ESPNOW
- **MavLinkControl** – MAVLink protocol implementation for drone control
- **LocalUI** – On-device menu system for configuration
- **RemoteUI** – Receives and displays data from remote devices

### Mode Switching
Press the **left joystick button** to cycle through available behaviors. The current mode is displayed on the OLED screen.

### Calibration
On first startup, joysticks are automatically calibrated (500 samples). Calibration can be modified via the `calibrate()` method in `Periphery.cpp`.

---

## Code Structure

```
ESP32-DJC/
├── DJC-Firmware/          # Main firmware
│   ├── src/
│   │   ├── main.cpp       # Application entry point
│   │   └── djc/           # Controller core logic
│   │       ├── Periphery.hpp      # Hardware pin mapping
│   │       ├── RemoteController.hpp # Main behavior manager
│   │       └── behaviors/ # Operational modes
│   │           ├── SimpleControl.hpp
│   │           ├── MavLinkControl.hpp
│   │           ├── LocalUI.hpp
│   │           └── RemoteUI.hpp
│   └── platformio.ini     # Build configuration
├── Images/                # Photos and schematics
├── Models/                # 3D printable case (STL)
└── README.md              # Documentation
```

---

## Communication Protocols

### ESPNOW
Default protocol for direct ESP32-to-ESP32 communication. The receiver MAC address is configured in `Periphery.hpp` (currently set to broadcast mode).

### MAVLink
For flight controller integration:
1. Ensure your flight controller supports MAVLink over serial or UDP
2. Adjust MAVLink system/component IDs in `MavLinkControl.hpp` if needed
3. The controller sends `MANUAL_CONTROL` messages and can receive telemetry data

---

## Customization

### Adding a New Behavior
1. Create a new `.hpp` file in `DJC-Firmware/src/djc/behaviors/`
2. Inherit from `kf::sys::Behavior` (refer to existing behaviors as examples)
3. Register it in `RemoteController.hpp` within the behaviors list
4. Rebuild and upload

### Changing Hardware Pins
Modify `Periphery.hpp` – update the GPIO numbers in the `Periphery` struct.

---

## Project Status

Active development. Core functionality is operational with some features in experimental state. Community contributions are welcome.

### Current Limitations
- ESPNOW pairing is currently hardcoded (broadcast address)
- MAVLink implementation covers basic manual control and heartbeat
- Case design is preliminary

---

## Contributing

Contributions are welcome:
- Report bugs or suggest features via [GitHub Issues](https://github.com/KiraFlux/ESP32-DJC/issues)
- Submit pull requests with improvements
- Share your modifications in the Discussions section

---

## License

This project is open-source under the MIT License. See the LICENSE file for details.

---

## Acknowledgments

- Built using the [KiraFlux library collection](https://github.com/KiraFlux)
- Inspired by the DIY RC and drone communities
- Thanks to all contributors and testers

---

If you create a project using this controller, please tag [@KiraFlux](https://github.com/KiraFlux) – we appreciate seeing your implementations.
