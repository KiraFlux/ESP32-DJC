// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/memory/StringView.hpp>
#include <kf/mixin/Callbacked.hpp>
#include <kf/ui/Block.hpp>
#include <kf/ui/Style.hpp>

#include "djc/transport/PeerAddress.hpp"

namespace djc::ui::widgets {

template<typename U> struct PeerDisplay :

    U::Widget,
    kf::mixin::Callbacked<const transport::PeerAddress &>

{
    struct State final {
        transport::PeerAddress address;
        kf::Option<kf::memory::StringView> name;

        kf::memory::StringView displayName() const noexcept {
            return name.isSome() ? name.unwrap().data() : address.toString().data();
        }
    };

    explicit constexpr PeerDisplay(kf::Option<State> state = kf::none, kf::ui::Style style = kf::ui::Style::defaults()) noexcept :
        U::Widget{style}, _state{state} {}

    void state(const kf::Option<State> &new_state) noexcept {
        _state = new_state;
    }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.beginBlock(kf::ui::Block::Alternative);
        if (_state.isSome()) {
            render.value(_state.unwrap().displayName());
        }
        render.endBlock(kf::ui::Block::Alternative);
    }

    bool onClick() noexcept override {
        if (_state.isSome()) {
            this->invoke(_state.unwrap().address);
        }
        return _state.isSome();
    }

private:
    kf::Option<State> _state;
};

}// namespace djc::ui::widgets