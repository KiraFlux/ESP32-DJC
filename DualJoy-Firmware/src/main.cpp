#include <Arduino.h>
#include <WiFi.h>

#include <KiraFlux-GUI.hpp>
#include <kf/Button.hpp>
#include <kf/Joystick.hpp>
#include <kf/JoystickListener.hpp>
#include <kf/Logger.hpp>
#include <kf/SSD1306.h>

#include <espnow/Protocol.hpp>
#include <rs/aliases.hpp>

#include "gui/FlagDisplay.hpp"
#include "gui/JoyWidget.hpp"

struct EspnowNode {
    espnow::Mac target;

    [[nodiscard]] bool init() const {
        if (not WiFiClass::mode(WIFI_MODE_STA)) {
            kf_Logger_error("WIFI mode set fail");
            return false;
        }

        if (const auto result = espnow::Protocol::init(); result.fail()) {
            kf_Logger_error(rs::toString(result.error));
            return false;
        }

        if (const auto result = espnow::Peer::add(target); result.fail()) {
            kf_Logger_error(rs::toString(result.error));
            return false;
        }

        kf_Logger_info("OK");
        return true;
    }

    template<typename T> inline rs::Result<void, espnow::Protocol::SendError> send(const T &value) {
        return espnow::Protocol::send(target, value);
    }
};

struct Periphery {
    kf::Button left_button{GPIO_NUM_15, kf::Button::Mode::PullUp};
    kf::Button right_button{GPIO_NUM_4, kf::Button::Mode::PullUp};
    kf::Joystick left_joystick{GPIO_NUM_32, GPIO_NUM_33, 0.5f};
    kf::Joystick right_joystick{GPIO_NUM_35, GPIO_NUM_34, 0.5f};
    kf::JoystickListener left_joystick_listener{left_joystick};
    EspnowNode espnow_node{{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
    kf::SSD1306 display_driver{};

    bool init() {
        kf_Logger_info("init");

        display_driver.init();
        Wire.setClock(1000000u);
        display_driver.update();

        left_joystick.init();
        right_joystick.init();
        left_button.init(false);
        right_button.init(false);

        if (not espnow_node.init()) {
            return false;
        }

        kf_Logger_info("OK");
        return true;
    }

    static Periphery &instance() {
        static Periphery instance;
        return instance;
    }
};

struct FlightBehavior : kfgui::Behavior {
    JoyWidget left_joy_widget;
    JoyWidget right_joy_widget;
    FlagDisplay toggle_mode;

    struct {
        float left_x{0}, left_y{0};
        float right_x{0}, right_y{0};
        bool toggle_mode{false};
    } packet{};

    void bindPainters(kf::Painter &root) noexcept override {
        auto [up, down] = root.splitVertically<2>({1, 7});
        auto [left_joy, right_joy] = down.splitHorizontally<2>({});

        left_joy_widget.painter = left_joy;
        right_joy_widget.painter = right_joy;
        toggle_mode.painter = up;
    }

    void loop() noexcept override {
        auto &periphery = Periphery::instance();

        packet.left_x = periphery.left_joystick.axis_x.read();
        packet.left_y = periphery.left_joystick.axis_y.read();
        packet.right_x = periphery.right_joystick.axis_x.read();
        packet.right_y = periphery.right_joystick.axis_y.read();

        periphery.left_button.poll();
        periphery.espnow_node.send(packet);
    }

    void onBind() noexcept override {
        auto &periphery = Periphery::instance();

        periphery.left_button.handler = [this]() {
            packet.toggle_mode = not packet.toggle_mode;
        };

        left_joy_widget.bindAxis(packet.left_x, packet.left_y);
        right_joy_widget.bindAxis(packet.right_x, packet.right_y);
        toggle_mode.flag = &packet.toggle_mode;
        toggle_mode.label = "mode";
    }

    static FlightBehavior &instance() {
        static FlightBehavior instance;
        return instance;
    }

private:
    FlightBehavior() {
        add(left_joy_widget);
        add(right_joy_widget);
        add(toggle_mode);
    }
};

struct RemoteMenuBehavior : kfgui::Behavior {
    enum class Code : rs::u8 {
        None = 0x00,
        Reload = 0x10,
        Click = 0x20,
        Left = 0x30,
        Right = 0x31,
        Up = 0x40,
        Down = 0x41
    };

    std::array<char, 250> text_buffer{"Waiting for menu..."};
    kfgui::TextDisplay text_display{text_buffer.data()};

    void bindPainters(kf::Painter &root) noexcept override {
        text_display.painter = root;
    }

    void loop() noexcept override {
        auto &periphery = Periphery::instance();
        periphery.left_joystick_listener.poll();
        periphery.left_button.poll();
    }

    void onBind() noexcept override {
        auto &periphery = Periphery::instance();

        periphery.left_joystick_listener.handler =
            [](kf::JoystickListener::Direction dir) {
                if (const auto code = translate(dir); code != Code::None) {
                    send(code);
                }
            };

        periphery.left_button.handler = []() { send(Code::Click); };

        espnow::Protocol::instance().setReceiveHandler(
            [](const espnow::Mac &mac, const void *data, rs::u8 size) {
                auto &self = RemoteMenuBehavior::instance();
                rs::size copy_size = std::min(size, static_cast<rs::u8>(self.text_buffer.size() - 1));
                memcpy(self.text_buffer.data(), data, copy_size);
                self.text_buffer[copy_size] = '\0';
            });

        send(Code::Reload);
    }

    static RemoteMenuBehavior &instance() {
        static RemoteMenuBehavior instance;
        return instance;
    }

private:
    static void send(Code code) { Periphery::instance().espnow_node.send(code); }

    static Code translate(kf::JoystickListener::Direction dir) {
        switch (dir) {
            case kf::JoystickListener::Direction::Up:
                return Code::Up;
            case kf::JoystickListener::Direction::Down:
                return Code::Down;
            case kf::JoystickListener::Direction::Left:
                return Code::Left;
            case kf::JoystickListener::Direction::Right:
                return Code::Right;
            default:
                return Code::None;
        }
    }

    RemoteMenuBehavior() { add(text_display); }
};

[[noreturn]] void setup() {
    static auto &periphery = Periphery::instance();
    static auto &manager = kfgui::BehaviorManager::instance();
    static auto &flight_behavior = FlightBehavior::instance();
    static auto &remote_menu_behavior = RemoteMenuBehavior::instance();

    Serial.begin(115200);
    kf::Logger::instance().write_func = [](const char *buffer, rs::size size) {
        Serial.write(buffer, size);
    };

    periphery.init();

    static kf::Painter painter{
        kf::FrameView{
            periphery.display_driver.buffer, kf::SSD1306::width,
            kf::SSD1306::width, kf::SSD1306::height, 0, 0},
        kf::fonts::gyver_5x7_en};

    painter.text("Init");
    periphery.display_driver.update();

    periphery.left_joystick.axis_x.inverted = true;
    periphery.right_joystick.axis_y.inverted = true;

    periphery.left_joystick.calibrate(500);
    periphery.right_joystick.calibrate(500);

    periphery.right_button.handler = []() {
        static std::array<kfgui::Behavior *, 2> behaviors{
            &flight_behavior,
            &remote_menu_behavior,
        };
        static rs::size current{0};

        manager.bindBehavior(*behaviors[current]);
        kf_Logger_debug("Behavior: %d", current);

        current += 1;
        current %= behaviors.size();
    };

    flight_behavior.bindPainters(painter);
    remote_menu_behavior.bindPainters(painter);

    manager.bindBehavior(flight_behavior);

    while (true) {
        manager.loop();
        periphery.right_button.poll();

        manager.render(painter);
        periphery.display_driver.update();

        delay(20);
    }
}

void loop() {}