// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/memory/Array.hpp>
#include <kf/memory/StaticString.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

struct PeerFavoritePage final : UI::Page {

    explicit PeerFavoritePage(UI &ui, UI::Page &root, PeerFavoritesRegistry &peer_favorites_registry) noexcept :
        Page{ui},
        _peer_favorites_registry{peer_favorites_registry},
        _description_input{ui.createTextInput()},
        _layout{{
            // address and transport shows in title
            &_labeled_trust_input,
            &_labeled_description_input,
            &_confirm_button,
            &root.link(),// quit without save
            &_delete_button,
        }}

    {
        _labeled_trust_input.hint("Set priority for auto connect");
        _labeled_description_input.hint("Will shown as human-readable alias");

        _confirm_button.hint("Write to registry");
        _confirm_button.callback([this]() -> void {
            if (_temp_entry.isNone()) { return; }
            _temp_entry.unwrap().trust = _trust_input.value();

            const bool write_ok = _peer_favorites_registry.put(_temp_entry.unwrap());
            _confirm_button.label(write_ok ? "Writed" : "Write failed");
            _confirm_button.style(UI::Style{
                .foreground_color = UI::Color::Normal,
                .background_color = (write_ok ? UI::Color::Success : UI::Color::Error),
            });

            _ui.requestRender();
        });

        _delete_button.hint("Remove from registry");
        _delete_button.callback([this, &root]() -> void {
            if (_temp_entry.isNone()) { return; }

            (void) _peer_favorites_registry.remove(_temp_entry.unwrap().address);

            _ui.activePage(root);
            _ui.requestRender();
        });
    }

    void bindPeer(const transport::PeerAddress &address) noexcept {
        const auto &entry_option = _peer_favorites_registry.get(address);

        _temp_entry = kf::someTrivial(entry_option.unwrapOr(PeerFavoritesRegistry::Entry::create(address)));

        (void) _label_buffer.format("%s Peer favorite\n%s", (entry_option.isSome() ? "Edit" : "Add"), address.toString().data());
        this->label(_label_buffer.view());

        _description_input.source({_temp_entry.unwrap().name.data(), _temp_entry.unwrap().name.size()});
        _trust_input.value(_temp_entry.unwrap().trust);
        _confirm_button.label("Confirm");
        _confirm_button.style({UI::Color::Primary, UI::Color::Normal});
        widgets(_layout.slice().first(_layout.size() - (entry_option.isSome() ? 0 : 1)));
    }

private:
    PeerFavoritesRegistry &_peer_favorites_registry;
    kf::TrivialOption<PeerFavoritesRegistry::Entry> _temp_entry{};

    kf::memory::StaticString<64> _label_buffer{};

    using TrustInput = UI::Slider<PeerFavoritesRegistry::Entry::TrustType>;

    TrustInput::Config _trust_input_config{
        .value_range = PeerFavoritesRegistry::Entry::trust_range,
        .default_value = PeerFavoritesRegistry::Entry::trust_range.start,
        .step = static_cast<PeerFavoritesRegistry::Entry::TrustType>(1),
        .init_show_value = true,
    };

    TrustInput _trust_input{_trust_input_config};
    UI::TextInput _description_input;

    UI::Labeled _labeled_trust_input{"Trust", _trust_input};
    UI::Labeled _labeled_description_input{"Name", _description_input};
    UI::Button _confirm_button{{}}, _delete_button{"Delete", UI::Style{.background_color = UI::Color::Danger}};
    kf::memory::Array<UI::Widget *, 5> _layout;
};

}// namespace djc::ui::pages