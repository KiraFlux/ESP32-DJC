#pragma once

#include <kf/sys.hpp>
#include <kf/espnow.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include "djc/Periphery.hpp"


namespace djc {

struct RemoteInterface : kf::sys::Behavior, kf::tools::Singleton<RemoteInterface> {
    friend struct Singleton<RemoteInterface>;

    enum class Code : kf::u8 {
        None = 0x00,
        Reload = 0x10,
        Click = 0x20,
        Left = 0x30,
        Right = 0x31,
        Up = 0x40,
        Down = 0x41
    };

    std::array<char, 250> text_buffer{"Waiting\nRemote Interface"};
    kf::sys::TextComponent text_display{text_buffer.data()};

    void updateLayout(kf::gfx::Canvas &root) override {
        text_display.canvas = root;
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
        
        periphery.espnow_node.on_receive = [this](const void *data, kf::u8 size) {
            const auto copy_size = std::min(size, static_cast<kf::u8>(text_buffer.size() - 1));
            std::memcpy(text_buffer.data(), data, copy_size);
            text_buffer[copy_size] = '\0';
        };

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
