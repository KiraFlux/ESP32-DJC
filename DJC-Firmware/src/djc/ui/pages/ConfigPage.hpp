// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/memory/Array.hpp>
#include <kf/memory/StaticString.hpp>

#include "djc/Config.hpp"
#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"
#include "djc/service/ConfigService.hpp"
#include "djc/transport/Kind.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/pages/PeerFavoritePage.hpp"

namespace djc::ui::pages {

struct ConfigPage : UI::Page {

    explicit ConfigPage(
        UI &ui,
        UI::Page &root,
        djc::service::ConfigService &config_service,
        PeerFavoritesRegistry &peer_favoriter_registry) noexcept :
        Page{ui},
        _config_service{config_service},
        _peer_favorite_page{ui, *this, _peer_favoriter_registry},
        _peer_favoriter_registry{peer_favoriter_registry},
        _device_name_input{ui.createTextInput()},
        _layout{{
            &root.link(),
            &_save_config_button,
            &_load_config_button,
            &_reset_config_button,
            &_device_name_input,
            &_labeled_autoconnect_enabled_input,
            &_labeled_default_transport_kind_selector,
            &_labeled_default_protocol_mode_selector,
            &_favorite_peers_fold_toggle_button,
        }} {
        this->label("Config");
        widgets(layout(0));

        this->link().hint("Open Configuration page");

        _device_name_input.hint("Device name");
        _device_name_input.source({_config_service.config().device_name.data(), _config_service.config().device_name.size()});

        _save_config_button.hint("Write config from RAM into NVS");
        _save_config_button.callback([this]() {
            _config_service.requestSave();
            _config_service.sync();
        });

        _load_config_button.hint("Load config from NVS into RAM");
        _load_config_button.callback([this]() {
            _config_service.requestLoad();
            _config_service.sync();
        });

        _reset_config_button.hint("Set RAM config as detaults");
        _reset_config_button.callback([this]() {
            _config_service.requestReset();
            _config_service.sync();
        });

        _favorite_peers_fold_toggle_button.hint("Toggle folding");
        _favorite_peers_fold_toggle_button.callback([this]() {
            show_favorites = not show_favorites;
            this->onEntry();
            _ui.requestRender();
        });

        _labeled_default_transport_kind_selector.hint("Define transport select after init");
        _default_transport_kind_selector.callback([this](auto item) {
            _config_service.config().init_transport_kind = item.value();
        });

        _labeled_default_protocol_mode_selector.hint("Define protocol select after init");
        _default_protocol_mode_selector.callback([this](auto item) {
            _config_service.config().init_protocol_mode = item.value();
            _config_service.requestSave();
        });

        _labeled_autoconnect_enabled_input.hint("Auto connect to most trusted peer");
        _autoconnect_enabled_input.callback([this](bool value) {
            _config_service.config().auto_connect_service.enabled = value;
            _config_service.requestSave();
        });

        for (auto i = 0u; i < _peer_favorite_displays.size(); i += 1) {
            auto &display = _peer_favorite_displays[i];
            _layout[layout_regular_widgets + i] = &display;

            display.hint("Open peer config");
            display.callback([this](const transport::PeerAddress &address) -> void {
                _peer_favorite_page.bindPeer(address);
                _ui.activePage(_peer_favorite_page);
            });
        }
    }

    void onEntry() noexcept override {
        const auto &config = _config_service.config();
        _default_protocol_mode_selector.value(config.init_protocol_mode);
        _default_transport_kind_selector.value(config.init_transport_kind);
        _autoconnect_enabled_input.value(config.auto_connect_service.enabled);

        const auto all_favorites = _peer_favoriter_registry.all();

        (void) _label_favorites_buffer.format(
            "[%c] Peer Favorites (%d/%d)",
            (show_favorites ? 'V' : '>'),
            all_favorites.size(),
            Config::max_peer_favorites);
        _favorite_peers_fold_toggle_button.label(_label_favorites_buffer.view());
        _favorite_peers_fold_toggle_button.background(show_favorites ? UI::Color::Secondary : UI::Color::Primary);

        if (show_favorites) {
            for (auto i = 0u; i < all_favorites.size(); i += 1) {
                const auto &favorite = all_favorites[i];
                if (favorite.isSome()) {
                    _peer_favorite_displays[i].state(kf::some(UI::PeerDisplay::State{
                        .address = favorite.unwrap().address,
                        .name = kf::some(kf::memory::StringView{favorite.unwrap().name.data(), favorite.unwrap().name.size()}),
                    }));
                    _peer_favorite_displays[i].foreground(UI::Color::Primary);
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

    // state

    djc::service::ConfigService &_config_service;
    PeerFavoritesRegistry &_peer_favoriter_registry;
    kf::memory::StaticString<32> _label_favorites_buffer{};
    bool show_favorites{true};

    // widgets

    kf::memory::Array<TransportKindSelector::Config::Item, 1> _transport_kind_options{{{
        {
            "EspNow",
            transport::Kind::EspNow,
            UI::Style{
                .foreground_color = UI::Color::Highlight,
            },
        },
    }}};

    TransportKindSelector::Config _transport_kind_config{
        .items = {_transport_kind_options.data(), _transport_kind_options.size()},
    };

    kf::memory::Array<ProtocolModeSelector::Config::Item, 2> _control_mode_options{{{
        {
            "Mavlink",
            Mode::Mavlink,
            UI::Style{
                .foreground_color = UI::Color::Highlight,
            },
        },
        {
            "Raw",
            Mode::Raw,
        },
    }}};

    ProtocolModeSelector::Config _control_mode_config{
        .items = {_control_mode_options.data(), _control_mode_options.size()},
    };

    UI::TextInput _device_name_input;

    UI::Button
        _save_config_button{
            "Save",
            UI::Style{
                .foreground_color = UI::Color::Primary,
            },
        },
        _load_config_button{
            "Load",
        },
        _reset_config_button{
            "Reset",
            UI::Style{
                .foreground_color = UI::Color::Danger,
            },
        },
        _favorite_peers_fold_toggle_button{{}};

    TransportKindSelector _default_transport_kind_selector{_transport_kind_config};
    UI::Labeled _labeled_default_transport_kind_selector{"Init Transport", _default_transport_kind_selector};

    ProtocolModeSelector _default_protocol_mode_selector{_control_mode_config};
    UI::Labeled _labeled_default_protocol_mode_selector{"Init Protocol", _default_protocol_mode_selector};

    kf::memory::Array<UI::PeerDisplay, Config::max_peer_favorites> _peer_favorite_displays{};

    UI::CheckBox _autoconnect_enabled_input{false};
    UI::Labeled _labeled_autoconnect_enabled_input{"Autoconnect", _autoconnect_enabled_input};

    // layout

    kf::memory::Array<UI::Widget *, (layout_regular_widgets + Config::max_peer_favorites)> _layout;

    // child pages

    PeerFavoritePage _peer_favorite_page;

    kf::Slice<UI::Widget *> layout(kf::usize displayed_peers) noexcept {
        return kf::Slice<UI::Widget *>{_layout.data(), _layout.size()}.first(layout_regular_widgets + displayed_peers);
    }
};

}// namespace djc::ui::pages