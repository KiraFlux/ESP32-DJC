// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/StringView.hpp>
#include <kf/mixin/Callbacked.hpp>

#include "djc/transport/PeerAddress.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::widgets {

struct PeerDisplay final : UI::Widget, kf::mixin::Callbacked<const transport::PeerAddress &> {

    enum class Color : char {
        Normal = '\xFC',
        Warn = '\xF9',
    };

    struct State final {
        transport::PeerAddress address;
        kf::Option<kf::memory::StringView> name;
        Color label_color;

        kf::memory::StringView displayName() const noexcept {
            return name.hasValue() ? name.value().data() : address.toString().data();
        }
    };

    void state(const kf::Option<State> &new_state) noexcept { _state = new_state; }

    void doRender(UI::RenderImpl &render) const noexcept override {
        render.beginAltBlock();

        if (_state.hasValue()) {
            render.value(
                kf::memory::ArrayString<64>::formatted(
                    "%c%s\x80",
                    static_cast<char>(_state.value().label_color),
                    _state.value().displayName())
                    .view());
        }

        render.endAltBlock();
    }

    bool onClick() noexcept override {
        if (_state.hasValue()) {
            this->invoke(_state.value().address);
        }
        return _state.hasValue();
    }

private:
    kf::Option<State> _state{};
};

}// namespace djc::ui::widgets