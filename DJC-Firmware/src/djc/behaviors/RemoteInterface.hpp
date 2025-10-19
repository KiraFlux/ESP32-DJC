#pragma once

#include <kf/sys.hpp>
#include <kf/espnow.hpp>

#include "djc/Periphery.hpp"
#include "djc/tools/Singleton.hpp"


namespace djc {

struct RemoteInterface : kf::sys::Behavior, Singleton<RemoteInterface> {
    friend struct Singleton<RemoteInterface>;

    enum class Code : rs::u8 {
        None = 0x00,
        Reload = 0x10,
        Click = 0x20,
        Left = 0x30,
        Right = 0x31,
        Up = 0x40,
        Down = 0x41
    };

    std::array<char, 250> text_buffer{"Waiting\nfor remote menu..."};
    kf::sys::TextComponent text_display{text_buffer.data()};

    void updateLayout(kf::gfx::Painter &root) override {
        text_display.painter = root;
    }

    void update() override {
        auto &periphery = djc::Periphery::instance();
        periphery.left_joystick_listener.poll();
        periphery.left_button.poll();
    }

    void onEntry() override {
        auto &periphery = djc::Periphery::instance();

        periphery.left_joystick_listener.handler = [](kf::JoystickListener::Direction dir) {
            if (const auto code = translate(dir); code != Code::None) {
                send(code);
            }
        };

        periphery.left_button.handler = []() { send(Code::Click); };

        kf::espnow::Protocol::instance().setReceiveHandler([this](const kf::espnow::Mac &mac, const void *data, rs::u8 size) {
            const auto copy_size = std::min(size, static_cast<rs::u8>(text_buffer.size() - 1));
            std::memcpy(text_buffer.data(), data, copy_size);
            text_buffer[copy_size] = '\0';
        });

        send(Code::Reload);
    }

private:
    static void send(Code code) {
        (void) djc::Periphery::instance().espnow_node.send(code);
    }

    static Code translate(kf::JoystickListener::Direction dir) {
        using D = kf::JoystickListener::Direction;

        switch (dir) {
            case D::Up:return Code::Up;
            case D::Down:return Code::Down;
            case D::Left:return Code::Left;
            case D::Right:return Code::Right;
            default:return Code::None;
        }
    }

    RemoteInterface() { addComponent(text_display); }
};

}// namespace djc
