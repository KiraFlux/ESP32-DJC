// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/aliases.hpp>
#include <kf/memory/Array.hpp>

#include "djc/Control.hpp"
#include "djc/InputHandler.hpp"
#include "djc/Periphery.hpp"
#include "djc/memory/Box.hpp"

namespace djc {

struct Config {

    struct PeerInfo {
        EspNow::Mac mac;
        kf::memory::Array<char, 10> info; 
    };

    using PeerFavoritesConfig = djc::memory::Box<PeerInfo, kf::u8, 8>;

    static constexpr auto latest_version{3};

    kf::u16 version;

    Periphery::Config periphery;
    InputHandler::Config input_handler;
    Control::Config control;
    PeerFavoritesConfig peer_favorites;

    [[nodiscard]] bool isLatestVersion() const noexcept { return version == latest_version; }

    static constexpr Config defaults() noexcept {
        return {
            .version = latest_version,
            .periphery = Periphery::Config::defaults(),
            .input_handler = InputHandler::Config::defaults(),
            .control = Control::Config::defaults(),
            .peer_favorites = PeerFavoritesConfig::defaults(),
        };
    }
};

}// namespace djc