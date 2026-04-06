// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/Logger.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/Slice.hpp>

#include "djc/ui/UI.hpp"
#include "djc/Control.hpp"
#include "djc/ui/widgets/PeerDisplay.hpp"

namespace djc::ui::pages {

/// @brief Main menu page for ESP32-DJC
struct PeersPage : UI::Page {

    explicit constexpr PeersPage(UI::Page &root, Control &control) noexcept :
        Page{"Peers"},
        _control{control},
        _layout{{
            &root.link(),
            &_peer_display,
        }}

    {
        widgets({_layout.data(), _layout.size()});
    }

    void onEntry() noexcept override {
        logger.debug("entry");

        _control.onReceiveFromUnknown([this](const Control::EspNow::Mac &mac, kf::memory::Slice<const kf::u8> data){
            logger.info(
                kf::memory::ArrayString<64>::formatted(
                    "Got %d bytes from %s",
                    data.size(),
                    Control::EspNow::stringFromMac(mac).data()
                )
            );

            _peer_display.update(mac, millis());

            UI::instance().addEvent(UI::Event::update());
        });
    }

    void onExit() noexcept override {
        logger.debug("exit");

        _control.onReceiveFromUnknown(Control::ReceiveFromUnknownCallback{nullptr});
    }

    void onUpdate(kf::math::Milliseconds now) noexcept override {
        _peer_display.checkForClear(now);
    }

private:
    static constexpr auto logger{kf::Logger::create("PeersPage")};

    Control &_control;

    // widgets

    widgets::PeerDisplay _peer_display{_control};

    // layout
    kf::memory::Array<UI::Widget *, 2> _layout;
};

}// namespace djc::ui::pages