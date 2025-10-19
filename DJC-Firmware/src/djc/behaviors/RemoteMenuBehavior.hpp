#pragma once

#include <KiraFlux-GUI.hpp>
#include <espnow/Protocol.hpp>

#include "djc/Periphery.hpp"
#include "djc/tools/Singleton.hpp"

namespace djc {

struct RemoteMenuBehavior : kfgui::Behavior, Singleton<RemoteMenuBehavior> {

    friend struct Singleton<RemoteMenuBehavior>;

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
        auto &periphery = djc::Periphery::instance();
        periphery.left_joystick_listener.poll();
        periphery.left_button.poll();
    }

    void onBind() noexcept override {
        auto &periphery = djc::Periphery::instance();

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

private:
    static void send(Code code) { djc::Periphery::instance().espnow_node.send(code); }

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

}// namespace djc
