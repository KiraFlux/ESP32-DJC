// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/aliases.hpp>
#include <kf/memory/Array.hpp>

#include "djc/Control.hpp"
#include "djc/InputHandler.hpp"
#include "djc/Periphery.hpp"

namespace djc {

struct Config {

    Periphery::Config periphery;
    InputHandler::Config input_handler;
    Control::Config control;

    static constexpr auto favorite_max{8u};
    kf::memory::Array<Control::EspNow::Mac, favorite_max> favorites;
    kf::u8 favorites_total, selected_favorite_index;// 0 .. favorite_max-1

    kf::u16 version;

    [[nodiscard]] constexpr const Control::EspNow::Mac& selectedFavorite() const noexcept {
        return favorites[selected_favorite_index];
    }

    static constexpr Config defaults() noexcept {
        return {
            .periphery = Periphery::Config::defaults(),
            .input_handler = InputHandler::Config::defaults(),
            .control = Control::Config::defaults(),
            .favorites = {
                {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},// broadcast
            },
            .favorites_total = 1,
            .selected_favorite_index = 0,// select broadcast as default
            .version = 1,
        };
    }
};

}// namespace djc