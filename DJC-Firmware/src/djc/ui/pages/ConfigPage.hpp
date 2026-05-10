// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>

#include "djc/Config.hpp"
#include "djc/ConfigManager.hpp"
#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/pages/PeerFavoritePage.hpp"
#include "djc/ui/widgets/PeerDisplay.hpp"
#include "djc/ui/widgets/TextInput.hpp"

namespace djc::ui::pages {

struct ConfigPage : UI::Page {

    explicit ConfigPage(UI::Page &root, PeerFavoritesRegistry &peer_favoriter_registry) noexcept :
        Page{"Config"},
        _peer_favoriter_registry{peer_favoriter_registry},
        _layout{{
            &root.link(),
            &_device_name_input,
            &_labeled_default_protocol_mode_selector,
            &_save_storage,
            &_load_storage,
            &_reset_storage,
            &_show_favorite_peers,
        }} {
        widgets(layout(0));

        _device_name_input.source({storage.config().device_name.data(), storage.config().device_name.size()});

        _save_storage.callback([]() {
            storage.save();
        });

        _load_storage.callback([]() {
            storage.load();
        });

        _reset_storage.callback([]() {
            storage.reset();
        });

        _show_favorite_peers.callback([this]() {
            show_favorites = not show_favorites;
            this->onEntry();
            UI::instance().addEvent(UI::Event::update());
        });
        _default_protocol_mode_selector.callback([](Mode mode) {
            storage.config().init_protocol_mode = mode;
        });

        for (auto i = 0u; i < _peer_favorite_displays.size(); i += 1) {
            _layout[layout_regular_widgets + i] = &_peer_favorite_displays[i];

            _peer_favorite_displays[i].callback([this](const transport::PeerAddress &address) -> void {
                _peer_favorite_page.bindPeer(address);
                UI::instance().bindPage(_peer_favorite_page);
            });
        }
    }

    void onEntry() noexcept override {
        const auto all_favorites = _peer_favoriter_registry.all();

        (void) _label_favorites_buffer.format(
            "[%c] Peer Favorites (%d/%d)",
            ((show_favorites) ? 'V' : '>'),
            all_favorites.size(),
            Config::max_peer_favorites);
        _show_favorite_peers.label(_label_favorites_buffer.view());

        if (show_favorites) {
            for (auto i = 0u; i < all_favorites.size(); i += 1) {
                const auto &favorite = all_favorites[i];
                if (favorite.hasValue()) {
                    _peer_favorite_displays[i].state({widgets::PeerDisplay::State{
                        .address = favorite.value().address,
                        .name = {{favorite.value().name.data(), favorite.value().name.size()}},
                        .label_color = widgets::PeerDisplay::Color::Normal,
                    }});
                }
            }
        }

        widgets(layout(show_favorites ? all_favorites.size() : 0));
    }

private:
    using Mode = protocol::ProtocolRegistry::Mode;
    using ProtocolModeSelector = UI::ComboBox<Mode>;

    static constexpr auto layout_regular_widgets{7u};

    inline static auto &storage{djc::ConfigManager::instance()};

    // state

    PeerFavoritesRegistry &_peer_favoriter_registry;
    kf::memory::ArrayString<32> _label_favorites_buffer{};
    bool show_favorites{true};

    // widgets

    kf::memory::Array<ProtocolModeSelector::Item, 2> _control_mode_options{{
        {protocol::ProtocolRegistry::stringFromMode(Mode::Mavlink), Mode::Mavlink},
        {protocol::ProtocolRegistry::stringFromMode(Mode::Raw), Mode::Raw},
    }};

    ProtocolModeSelector::Config _control_mode_config{
        .items = {_control_mode_options.data(), _control_mode_options.size()},
    };

    widgets::TextInput _device_name_input{};

    UI::Button
        _save_storage{"Save"},
        _load_storage{"Load"},
        _reset_storage{"Reset (RAM cache)"},
        _show_favorite_peers{{}};

    ProtocolModeSelector _default_protocol_mode_selector{_control_mode_config};// TODO: set init value from storage

    UI::Labeled _labeled_default_protocol_mode_selector{"Init Protocol", _default_protocol_mode_selector};

    kf::memory::Array<widgets::PeerDisplay, Config::max_peer_favorites> _peer_favorite_displays{};

    // layout

    kf::memory::Array<UI::Widget *, (layout_regular_widgets + Config::max_peer_favorites)> _layout;

    // child pages

    PeerFavoritePage _peer_favorite_page{*this, _peer_favoriter_registry};

    kf::memory::Slice<UI::Widget *> layout(kf::usize displayed_peers) noexcept {
        return kf::memory::Slice<UI::Widget *>{_layout.data(), _layout.size()}.first(layout_regular_widgets + displayed_peers);
    }
};

}// namespace djc::ui::pages