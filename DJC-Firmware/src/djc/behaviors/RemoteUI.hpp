#pragma once

#include <cstring>

#include <kf/EspNow.hpp>
#include <kf/sys.hpp>
#include <kf/tools/meta/Singleton.hpp>
#include <kf/ui/Event.hpp>

#include "djc/Periphery.hpp"

namespace djc {

struct RemoteUI : kf::sys::Behavior, kf::tools::Singleton<RemoteUI> {
    friend struct Singleton<RemoteUI>;

    using Event = kf::ui::Event;

    std::array<char, 250> text_buffer{"Remote UI:\nIn Waiting..."};
    kf::sys::TextComponent text_display{text_buffer.data()};

    void updateLayout(kf::gfx::Canvas &root) override {
        text_display.canvas = root;
    }

    void update() override {
        auto &periphery = djc::Periphery::instance();
        periphery.right_button.poll();
        periphery.joystick_listener.poll();
    }

    void onEntry() override {
        auto &periphery = djc::Periphery::instance();

        periphery.right_button.handler = []() {
            send(Event{Event::Type::WidgetClick});
        };

        periphery.joystick_listener.handler = [](kf::JoystickListener::Direction dir) {
            using D = kf::JoystickListener::Direction;
            auto translate = [](D direction) -> Event {
                switch (direction) {
                    case D::Up: return Event::PageCursorMove(-1);
                    case D::Down: return Event::PageCursorMove(+1);
                    case D::Left: return Event::WidgetValueChange(+1);
                    case D::Right: return Event::WidgetValueChange(-1);
                    default: return Event::None();
                }
            };
            const auto event = translate(dir);
            if (Event::Type::None != event.type()) {
                send(event);
            }
        };

        kf::EspNow::instance().setUnknownReceiveHandler([this](const kf::EspNow::Mac &, const kf::slice<const void> &buffer) {
            const auto copy_size = std::min(buffer.size(), (text_buffer.size() - 1));
            std::memcpy(text_buffer.data(), buffer.data(), copy_size);
            text_buffer[copy_size] = '\0';
        });

        send(Event{Event::Type::Update});
    }

private:
    static void send(Event event) {
        (void) djc::Periphery::instance().espnow_peer.value().sendPacket(event);
    }

    RemoteUI() {
        addComponent(text_display);
    }
};

}// namespace djc
