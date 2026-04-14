// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/Logger.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/Slice.hpp>

#include "djc/Control.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/widgets/TextInput.hpp"

namespace djc::ui::pages {

struct RawControlPage : UI::Page {
    explicit RawControlPage(UI::Page &root, Control &control) noexcept :
        Page{"Raw Control"}, _control{control},
        _layout{{
            &root.link(),
            &_message_input,
            &_send_button,
        }} {
        widgets({_layout.data(), _layout.size()});

        _send_button.callback([this]() {            
            kf::memory::StringView s{_message.data(), _message.size()};
            s = s.sub(0, s.find('\0').valueOr(s.size()));

            logger.debug(s);

            _control.sendRawMessage({reinterpret_cast<const kf::u8 *>(s.data()), s.size()});
        });
    }

    void onEntry() noexcept override {
        _control.mode(Control::Mode::Raw);
        _control.onRawMessage([](kf::memory::Slice<const kf::u8> buffer) {
            logger.info(
                kf::memory::ArrayString<64>::formatted(
                    "Got %d bytes from primary peer",
                    buffer.size())
                    .view());
        });
    }

    void onExit() noexcept override {
        _control.onRawMessage(Control::RawMessageCallback{nullptr});
    }

private:
    static constexpr auto logger{kf::Logger::create("RawControlPage")};

    kf::memory::Array<char, 200> _message{};
    Control &_control;

    // widgets

    widgets::TextInput _message_input{{_message.data(), _message.size()}};
    UI::Button _send_button{"Send"};

    kf::memory::Array<UI::Widget *, 3> _layout;
};

}// namespace djc::ui::pages