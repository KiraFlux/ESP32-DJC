// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/widgets/TextInput.hpp"

namespace djc::ui::pages {

struct PeerFavoritePage final : UI::Page {

    explicit PeerFavoritePage(UI::Page &root, PeerFavoritesRegistry &peer_favorites_registry) noexcept :
        Page{{}},
        _peer_favorites_registry{peer_favorites_registry},
        _layout{{
            // address and transport shows in title
            &_labeled_trust_input,
            &_labeled_description_input,
            &_confirm_button,
            &root.link(),// quit without save
            &_delete_button,
        }}

    {
        _confirm_button.callback([this]() -> void {
            if (not _temp_entry.hasValue()) { return; }
            _temp_entry.value().trust = _trust_input.value();

            const auto result = _peer_favorites_registry.put(_temp_entry.value());
            _confirm_button.label(result ? "Writed" : "Write failed");

            UI::instance().addEvent(UI::Event::update());
        });

        _delete_button.callback([this, &root]() -> void {
            if (not _temp_entry.hasValue()) { return; }

            (void) _peer_favorites_registry.remove(_temp_entry.value().address);

            UI::instance().bindPage(root);
            UI::instance().addEvent(UI::Event::update());
        });
    }

    void bindPeer(const transport::PeerAddress &address) noexcept {
        const auto &entry_option = _peer_favorites_registry.get(address);

        _temp_entry.value(entry_option.valueOr(PeerFavoritesRegistry::Entry::create(address)));

        (void) _label_buffer.format("%s Peer favorite\n%s", (entry_option.hasValue() ? "Edit" : "Add"), address.toString().data());
        this->label(_label_buffer.view());

        _description_input.source({_temp_entry.value().name.data(), _temp_entry.value().name.size()});
        _trust_input.value(_temp_entry.value().trust);
        _confirm_button.label("Confirm");

        widgets(kf::memory::Slice<UI::Widget *>{_layout.data(), _layout.size()}.first(_layout.size() - (entry_option.hasValue() ? 0 : 1)));
    }

private:
    PeerFavoritesRegistry &_peer_favorites_registry;
    kf::Option<PeerFavoritesRegistry::Entry> _temp_entry{};

    kf::memory::ArrayString<64> _label_buffer{};

    using TrustInput = UI::Slider<PeerFavoritesRegistry::Entry::TrustType>;

    TrustInput::Config _trust_input_config{
        .value_range = PeerFavoritesRegistry::Entry::trust_range,
        .default_value = PeerFavoritesRegistry::Entry::trust_range.start,
        .step = static_cast<PeerFavoritesRegistry::Entry::TrustType>(1),
        .placement = UI::Placement::Outside,
        .init_show_value = true,
    };

    TrustInput _trust_input{_trust_input_config};
    widgets::TextInput _description_input{};

    UI::Labeled _labeled_trust_input{"Trust", _trust_input};
    UI::Labeled _labeled_description_input{"Name", _description_input};
    UI::Button _confirm_button{{}}, _delete_button{"\xF9"
                                                   "Delete\x80"};
    kf::memory::Array<UI::Widget *, 5> _layout;
};

}// namespace djc::ui::pages