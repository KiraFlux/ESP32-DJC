// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/memory/StringView.hpp>
#include <kf/mixin/Callbacked.hpp>
#include <kf/ui/widgets/Widget.hpp>

#include "djc/transport/PeerAddress.hpp"

namespace djc::ui::widgets {

template<typename U> struct PeerDisplay :

    kf::ui::widgets::Widget<U>,
    kf::mixin::Callbacked<const transport::PeerAddress &>

{
    struct State final {
        transport::PeerAddress address;
        kf::Option<kf::memory::StringView> name;

        kf::memory::StringView displayName() const noexcept {
            return name.isSome() ? name.unwrap().data() : address.toString().data();
        }
    };

    void state(const kf::Option<State> &new_state) noexcept {
        _state = new_state;
    }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.beginAltBlock();
        if (_state.isSome()) {
            render.value(_state.unwrap().displayName());
        }
        render.endAltBlock();
    }

    bool onClick() noexcept override {
        if (_state.isSome()) {
            this->invoke(_state.unwrap().address);
        }
        return _state.isSome();
    }

private:
    kf::Option<State> _state{};
};

}// namespace djc::ui::widgets