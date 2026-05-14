// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/mixin/Initable.hpp>

#include "djc/Config.hpp"
#include "djc/ConfigManager.hpp"
#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"
#include "djc/transport/Kind.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/pages/PeerFavoritePage.hpp"
#include "djc/ui/widgets/PeerDisplay.hpp"
#include "djc/ui/widgets/TextInput.hpp"

namespace djc::ui::pages {

struct ConfigPage : UI::Page, kf::mixin::Initable<ConfigPage, void> {

    explicit ConfigPage(UI::Page &root, PeerFavoritesRegistry &peer_favoriter_registry) noexcept :
        Page{"Config"},
        _peer_favoriter_registry{peer_favoriter_registry},
        _layout{{
            &root.link(),
            &_device_name_input,
            &_labeled_autoconnect_enabled_input,
            &_labeled_default_transport_kind_selector,
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

        _autoconnect_enabled_input.callback([](bool value) {
            storage.config().auto_connect_service.enabled = value;
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
    using TransportKindSelector = UI::ComboBox<transport::Kind>;

    using Mode = protocol::ProtocolRegistry::Mode;
    using ProtocolModeSelector = UI::ComboBox<Mode>;

    static constexpr auto layout_regular_widgets{9u};

    inline static auto &storage{djc::ConfigManager::instance()};

    // state

    PeerFavoritesRegistry &_peer_favoriter_registry;
    kf::memory::ArrayString<32> _label_favorites_buffer{};
    bool show_favorites{true};

    // widgets

    kf::memory::Array<TransportKindSelector::Item, 1> _transport_kind_options{{
        {"EspNow", transport::Kind::EspNow},
    }};

    TransportKindSelector::Config _transport_kind_config{
        .items = {_transport_kind_options.data(), _transport_kind_options.size()},
    };

    kf::memory::Array<ProtocolModeSelector::Item, 2> _control_mode_options{{
        {"Mavlink", Mode::Mavlink},
        {"Raw", Mode::Raw},
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

    TransportKindSelector _default_transport_kind_selector{_transport_kind_config};
    UI::Labeled _labeled_default_transport_kind_selector{"Init Transport", _default_transport_kind_selector};

    ProtocolModeSelector _default_protocol_mode_selector{_control_mode_config};
    UI::Labeled _labeled_default_protocol_mode_selector{"Init Protocol", _default_protocol_mode_selector};

    kf::memory::Array<widgets::PeerDisplay, Config::max_peer_favorites> _peer_favorite_displays{};

    UI::CheckBox _autoconnect_enabled_input{false};
    UI::Labeled _labeled_autoconnect_enabled_input{"Autoconnect", _autoconnect_enabled_input};

    // layout

    kf::memory::Array<UI::Widget *, (layout_regular_widgets + Config::max_peer_favorites)> _layout;

    // child pages

    PeerFavoritePage _peer_favorite_page{*this, _peer_favoriter_registry};

    kf::memory::Slice<UI::Widget *> layout(kf::usize displayed_peers) noexcept {
        return kf::memory::Slice<UI::Widget *>{_layout.data(), _layout.size()}.first(layout_regular_widgets + displayed_peers);
    }

    // impl
    KF_IMPL_INITABLE(ConfigPage, void);
    void initImpl() noexcept {
        // _default_protocol_mode_selector.value(storage.config().init_protocol_mode); // todo: Combobox::value(T)
        _autoconnect_enabled_input.value(storage.config().auto_connect_service.enabled);
    }
};

}// namespace djc::ui::pages