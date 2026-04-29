// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/aliases.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/Control.hpp"
#include "djc/PeerScanner.hpp"
#include "djc/Periphery.hpp"
#include "djc/input/InputHandler.hpp"
#include "djc/memory/Box.hpp"
#include "djc/transport/PeerAddress.hpp"

namespace djc {

struct Config {

    struct PeerNote {
        char address;// temp
        kf::memory::Array<char, 10> info;
    };

    using PeerFavoritesConfig = djc::memory::Box<PeerNote, kf::u8, 8>;

    static constexpr auto latest_version{5};

    kf::u16 version;

    Periphery::Config periphery;
    InputHandler::Config input_handler;
    Control::Config control;
    PeerScanner::Config peer_scanner;
    PeerFavoritesConfig peer_favorites;
    kf::memory::Array<char, 16> device_name;

    [[nodiscard]] constexpr kf::memory::StringView deviceName() const noexcept {
        return kf::memory::StringView{device_name.data(), device_name.size()};
    }

    [[nodiscard]] bool isLatestVersion() const noexcept { return version == latest_version; }

    static constexpr Config defaults() noexcept {
        return Config{
            .version = latest_version,
            .periphery = Periphery::Config::defaults(),
            .input_handler = InputHandler::Config::defaults(),
            .control = Control::Config::defaults(),
            .peer_scanner = PeerScanner::Config::defaults(),
            .peer_favorites = PeerFavoritesConfig::defaults(),
            .device_name = {"ESP32-DJC"},
        };
    }
};

}// namespace djc