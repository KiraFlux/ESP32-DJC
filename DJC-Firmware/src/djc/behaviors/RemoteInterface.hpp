#pragma once

#include <kf/sys.hpp>
#include <kf/JoystickListener.hpp>
#include <kf/EspNow.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include "djc/Periphery.hpp"


namespace djc {

struct RemoteInterface : kf::sys::Behavior, kf::tools::Singleton<RemoteInterface> {
    friend struct Singleton<RemoteInterface>;

    /// @brief Обработчик дискретного ввода джойстика
    kf::JoystickListener joystick_listener{Periphery::instance().right_joystick};

    std::array<char, 250> text_buffer{"Waiting\nRemote Interface"};
    kf::sys::TextComponent text_display{text_buffer.data()};

    void updateLayout(kf::gfx::Canvas &root) override {
        text_display.canvas = root;
    }

    void update() override {
        auto &periphery = djc::Periphery::instance();
        periphery.right_button.poll();

        joystick_listener.poll();
    }

    void onEntry() override {
        auto &periphery = djc::Periphery::instance();

        periphery.right_button.handler = []() {
            send(Code::Click);
        };

        kf::EspNow::instance().setUnknownReceiveHandler([this](const kf::EspNow::Mac &, const kf::slice<const void> &buffer) {
            const auto copy_size = std::min(buffer.size, (text_buffer.size() - 1));
            std::memcpy(text_buffer.data(), buffer.ptr, copy_size);
            text_buffer[copy_size] = '\0';
        });

        send(Code::Reload);
    }

private:
    enum class Code : kf::u8 {
        None = 0x00,
        Reload = 0x10,
        Click = 0x20,
        Left = 0x30,
        Right = 0x31,
        Up = 0x40,
        Down = 0x41
    };

    static void send(Code code) {
        (void) djc::Periphery::instance().espnow_peer.value().sendPacket(code);
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

    RemoteInterface() {
        addComponent(text_display);

        joystick_listener.handler = [](kf::JoystickListener::Direction dir) {
            const auto code = translate(dir);

            if (code != Code::None) {
                send(code);
            }
        };
    }
};

}// namespace djc
