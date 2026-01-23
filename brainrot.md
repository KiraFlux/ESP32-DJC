# ESP32 Dual Joystick Controller (DJC)

An open-source, customizable remote controller based on ESP32 with dual analog joysticks, display support, and ESPNOW
communication. Designed for DIY drone, rover, and robotics projects.

---

## Features

- **Dual Analog Joysticks** with push-button functionality
- **Display Support**: Compatible with SSD1306 (I2C) or ST7735 (SPI) displays
- **ESPNOW Communication**: Low-latency, direct ESP32-to-ESP32 control
- **MAVLink Protocol**: Compatibility with ArduPilot, PX4, and other flight controllers
- **Modular UI System**: Interactive menus with widget-based interface
- **3D-Printable Enclosure**: Designed for easy assembly
- **Powerful Toolkit**: Built on the [KiraFlux-Toolkit](https://github.com/KiraFlux) library

---

## Hardware

### Default Pin Mapping (`Periphery.hpp`)

| Component          | GPIO    | Function |
|--------------------|---------|----------|
| **Left Joystick**  | GPIO 32 | X-axis   |
|                    | GPIO 33 | Y-axis   |
|                    | GPIO 14 | Button   |
| **Right Joystick** | GPIO 35 | X-axis   |
|                    | GPIO 34 | Y-axis   |
|                    | GPIO 4  | Button   |
| **Display (I2C)**  | GPIO 21 | SDA      |
|                    | GPIO 22 | SCL      |
| **Display (SPI)**  | GPIO 23 | MOSI     |
|                    | GPIO 19 | MISO     |
|                    | GPIO 18 | SCK      |
|                    | GPIO 5  | CS       |
|                    | GPIO 2  | DC       |
|                    | GPIO 15 | Reset    |

---

## Software Setup

### 1. Clone with Submodules

```bash
git clone --recursive https://github.com/KiraFlux/ESP32-DJC.git
cd ESP32-DJC
```

### 2. Open in PlatformIO

- Install [PlatformIO for VS Code](https://platformio.org/install/ide?install=vscode)
- Open the project folder in VS Code
- PlatformIO will install all dependencies automatically

### 3. Build and Upload

- Connect your ESP32 via USB
- Click **Build** (✓) then **Upload** (→) in PlatformIO
- Monitor serial output at 115200 baud

---

## Code Architecture

```
ESP32-DJC/
├── src/
│   ├── main.cpp              # Application entry point
│   └── djc/
│       ├── Periphery.hpp     # Hardware abstraction
│       ├── UI.hpp           # UI system configuration
│       └── ui/              # UI pages
│           ├── MainPage.hpp
│           ├── MavLinkControlPage.hpp
│           └── TestPage.hpp
├── Models/                   # 3D printable enclosure
├── Images/                   # Photos and schematics
└── README.md
```

### Key Components

- **`Periphery`**: Singleton managing all hardware (joysticks, buttons, display, ESPNOW)
- **`UI`**: Text-based UI system with widget support
- **`Page`**: Base class for all UI pages with lifecycle methods (`onEntry`, `onUpdate`, `onExit`)

### Adding New Pages

1. Create a new `.hpp` file in `src/djc/ui/`
2. Inherit from `UI::Page`
3. Implement required methods
4. Register in `main.cpp`

---

## Usage

### Controls

- **Left Joystick Button**: Toggles menu navigation mode
- **Right Joystick**: Navigates menus (when navigation enabled)
- **Right Button**: Selects/activates UI widgets

### Navigation Modes

- **Menu Mode**: Right joystick navigates UI pages
- **Control Mode**: Right joystick controls the remote device (drone/robot)

### Communication

- **ESPNOW**: Default for direct ESP32-to-ESP32 communication
- **MAVLink**: For flight controller integration (manual control, heartbeat)

---

## 3D Models and Production

The project uses [Scaffold](https://github.com/KiraFlux/Scaffold) for managing 3D models and production artifacts.

### Model Structure

```
Models/
├── Enclosure/           # 3D printable case parts
├── Panels/             # Laser-cut panels
└── Hardware/           # Mounting solutions
```

### Generating Artifacts

```bash
# Coming soon: Generation scripts
```

---

## Dependencies

- **[KiraFlux-Toolkit](https://github.com/KiraFlux)**: Core library collection
- **MAVLink**: For flight controller communication
- **Arduino-ESP32**: ESP32 support

---

## Project Status

**Active Development** - Core functionality stable, UI system evolving.

### Current Features

- ✅ Dual joystick input with calibration
- ✅ OLED display support (SSD1306/ST7735)
- ✅ ESPNOW communication
- ✅ MAVLink protocol support
- ✅ Widget-based UI system

### Planned Features

- Bluetooth support
- Advanced telemetry display
- Custom widget development
- Configuration storage

---

## Contributing

Contributions are welcome! Please:

1. Report bugs via [GitHub Issues](https://github.com/KiraFlux/ESP32-DJC/issues)
2. Submit pull requests with improvements
3. Share your modifications in Discussions

---

## License

MIT License - see [LICENSE](LICENSE) for details.

---

## Acknowledgments

- Built using the [KiraFlux-Toolkit](https://github.com/KiraFlux) library
- Inspired by the DIY RC and robotics communities
- Thanks to all contributors and testers

---

*If you build a project using ESP32-DJC, please tag [@KiraFlux](https://github.com/KiraFlux) – we'd love to see your
creations!*