// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/Logger.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/Slice.hpp>

#include "djc/Control.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

struct RawControlPage : UI::Page {
    explicit RawControlPage(UI::Page &root, Control &control) noexcept :
        Page{"Raw Control"}, _control{control},
        _layout{{
            &root.link(),
        }} {
        widgets({_layout.data(), _layout.size()});
    }

    void onEntry() noexcept override {
        logger.debug("entry");

        _control.mode(Control::Mode::Raw);
        _control.onRawMessage([](kf::memory::Slice<const kf::u8> buffer) {
            logger.info(
                kf::memory::ArrayString<64>::formatted(
                    "Got %d bytes from primaty peer",
                    buffer.size())
                    .view());
        });
    }

    void onExit() noexcept override {
        logger.debug("exit");

        _control.onRawMessage(Control::RawMessageCallback{nullptr});
    }

private:
    static constexpr auto logger{kf::Logger::create("RawControlPage")};

    Control &_control;
    kf::memory::Array<UI::Widget *, 1> _layout;
};

}// namespace djc::ui::pages