// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/algorithm.hpp"
#include "kf/math/units.hpp"
#include "kf/memory/Queue.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/mixin/Labeled.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/TimedPollable.hpp"
#include "kf/primitives.hpp"

#include "kf/ui/UiTraits.hpp"
#include "kf/ui/render/Render.hpp"
#include "kf/ui/widgets/Button.hpp"
#include "kf/ui/widgets/CheckBox.hpp"
#include "kf/ui/widgets/ComboBox.hpp"
#include "kf/ui/widgets/Display.hpp"
#include "kf/ui/widgets/Labeled.hpp"
#include "kf/ui/widgets/Slider.hpp"
#include "kf/ui/widgets/SpinBox.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui {

/// @brief User interface framework with widget-based rendering
/// @tparam U UI Traits implementation
template<typename U> struct UI :

    mixin::NonCopyable,
    mixin::TimedPollable<UI<U>>

{
    KF_CHECK_IMPL(U, ::kf::ui::UiTraitsTag);

    /// @brief UI Traits implementation
    struct Traits : U {};

    using Widget = widgets::Widget<Traits>;

    /// @brief Button widget with click handler
    struct Button : widgets::Button<Traits> {
        using widgets::Button<Traits>::Button;
    };

    /// @brief Checkbox with boolean state and change handler
    struct CheckBox : widgets::CheckBox<Traits> {
        using widgets::CheckBox<Traits>::CheckBox;
    };

    /// @brief Combo box for selecting from a fixed set of options
    /// @tparam T Value type of options
    template<typename T> struct ComboBox : widgets::ComboBox<Traits, T> {
        using widgets::ComboBox<Traits, T>::ComboBox;
    };

    /// @brief Read-only display of a value (by reference)
    /// @tparam T Type of displayed value (must outlive the widget)
    template<typename T> struct Display : widgets::Display<Traits, T> {
        using widgets::Display<Traits, T>::Display;
    };

    /// @brief Wrapper that adds a label to another widget
    struct Labeled : widgets::Labeled<Traits> {
        using widgets::Labeled<Traits>::Labeled;
    };

    /// @brief Spin box for numeric adjustment with configurable step mode
    /// @tparam T Arithmetic type (int, float, etc.)
    /// @tparam A Adjuster (Arithmetic, ArithmeticPositiveOnly, Geometric)
    template<typename T, typename A> struct SpinBox : widgets::SpinBox<Traits, T, A> {
        using widgets::SpinBox<Traits, T, A>::SpinBox;
    };

    /// @brief Slider for numeric adjustment with constraints
    /// @tparam T Arithmetic type (int, float, etc.)
    template<typename T> struct Slider : widgets::Slider<Traits, T> {
        using widgets::Slider<Traits, T>::Slider;
    };

    struct Page;// forward declaration

    explicit constexpr UI(typename Traits::RenderImpl &render_system) noexcept : _render_system{render_system} {}

    /// @brief Set active page
    /// @param page Page to make active (must remain valid)
    void activePage(Page &page) noexcept {
        if (_active_page.isSome()) {
            _active_page.unwrap().onExit();
        }
        page.onEntry();
        _active_page = someRef(page);
    }

    /// @brief Get readobly access to active page
    auto activePage() const noexcept -> kf::Option<Page &> {
        return _active_page;
    }

    /// @brief Add event to processing queue
    /// @note If the queue is non‑empty and the last event is Update, adding another Update event is a no‑op.
    void addEvent(typename Traits::EventImpl event) {
        using Kind = typename Traits::EventImpl::Kind;
        if (not _events.empty() and Kind::Update == event.kind() and Kind::Update == _events.back().kind()) { return; }
        _events.push(event);
    }

private:
    /// @brief Special widget for creating page navigation buttons
    /// @note Internal use only - use Page::link() for page navigation
    struct PageSetter : Widget {
        explicit PageSetter(Page &target) noexcept : _target{target} {
            this->hint("Navigate to page..");
        }

        /// @brief Set target page as active on click
        [[nodiscard]] bool onClick() noexcept override {
            _target._ui.activePage(_target);
            return true;// redraw always required after page change
        }

        void doRender(typename Traits::RenderImpl &render) const noexcept override {
            render.arrow();
            render.value(_target.label());
        }

    private:
        Page &_target;///< Target page for navigation
    };

public:
    /// @brief UI page containing widgets and label
    struct Page : mixin::NonCopyable, mixin::Labeled {
        friend struct PageSetter;

        explicit constexpr Page(UI &ui, memory::StringView label) noexcept : mixin::Labeled::Labeled{label}, _ui{ui} {}

        /// @brief Page behavior on entry
        virtual void onEntry() noexcept {}

        /// @brief Page behavior on leave
        virtual void onExit() noexcept {}

        /// @brief Page behavior on UI polling
        virtual void onUpdate(math::Milliseconds now) noexcept {}

        /// @brief Get 'go to this page' Widget
        [[nodiscard]] constexpr Widget &link() noexcept {
            return _to_this;
        }

        /// @brief Get selected widget
        [[nodiscard]] constexpr auto selectedWidget() const noexcept -> Option<const Widget &> {
            return _widgets.empty() ? none : someRef<const Widget &>(*_widgets[_cursor]);
        }

        /// @brief Render page content to display.
        /// @param render Renderer instance.
        /// @note Handles cursor positioning and widget focus.
        void render(typename Traits::RenderImpl &render) noexcept {
            render.title(this->label());

            if (nullptr == _widgets.data()) { return; }

            const usize available = render.widgetsAvailable();
            const usize start = (_widgets.size() > available) ? kf::min(static_cast<usize>(_cursor), _widgets.size() - available) : 0;
            const usize end = kf::min(start + available, _widgets.size());

            for (auto i = start; i < end; i += 1) {
                auto widget = _widgets[i];

                if (nullptr == widget) { continue; }

                widget->render(render, i, (i == _cursor));
            }
        }

        /// @brief Add event to processing queue.
        /// @param event The event to be processed.
        [[nodiscard]] bool onEvent(typename Traits::EventImpl event) noexcept {
            using Kind = typename Traits::EventImpl::Kind;
            switch (event.kind()) {
                case Kind::Update: {
                    return true;
                }
                case Kind::PageCursorMove: {
                    return moveCursor(event.value());
                }
                case Kind::WidgetClick: {
                    if (_widgets.size() > 0) {
                        return _widgets[_cursor]->onClick();
                    }
                }
                case Kind::WidgetValue: {
                    if (_widgets.size() > 0) {
                        return _widgets[_cursor]->onEventValue(event.value());
                    }
                }
            }
            return false;
        }

        /// @brief Get widgets on page
        [[nodiscard]] auto widgets() noexcept -> Slice<Widget *> {
            return _widgets;
        }

    protected:
        /// @brief Set widgets on page
        void widgets(Slice<Widget *> new_widgets) noexcept {
            _widgets = new_widgets;
            _cursor = clamp<isize>(_cursor, 0, _widgets.size() - 1);
        }

        /// @brief Add Update event
        void update() noexcept {
            _ui.addEvent(Traits::EventImpl::update());
        }

        UI &_ui;

    private:
        PageSetter _to_this{*this};///< Navigation widget to this page
        Slice<Widget *> _widgets{};///< Widgets on this page
        isize _cursor{0};          ///< Current widget cursor position (focused widget index)

        /// @brief Move cursor within page bounds
        /// @param delta Cursor movement delta (positive/negative)
        /// @return true if cursor position changed (redraw required)
        [[nodiscard]] bool moveCursor(isize delta) noexcept {
            const auto n = _widgets.size();
            if (n > 1) {
                _cursor = (_cursor + delta + n) % n;
                return true;
            } else {
                return false;
            }
        }
    };

private:
    memory::Queue<typename Traits::EventImpl> _events{};///< Event queue for pending UI events
    typename Traits::RenderImpl &_render_system;        ///< Renderer system implementation
    Option<Page &> _active_page{none};                  ///< Currently active page for rendering

    using This = UI<U>;

    KF_IMPL_TIMED_POLLABLE(This);
    // Process active page update, pending events and render if needed
    void pollImpl(math::Milliseconds now) noexcept {
        if (_active_page.isNone()) { return; }

        _active_page.unwrap().onUpdate(now);

        if (_events.empty()) { return; }

        constexpr usize max_events_per_poll{20};
        usize events_processed{0};

        bool render_required{false};

        while (not _events.empty() and events_processed < max_events_per_poll) {
            render_required |= _active_page.unwrap().onEvent(_events.front());
            events_processed += 1;
            _events.pop();
        }

        if (render_required) {
            _render_system.beginFrame();
            _active_page.unwrap().render(_render_system);
            _render_system.endFrame();
        }
    }
};

}// namespace kf::ui