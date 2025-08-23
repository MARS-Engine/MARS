#pragma once

#include "mars/math/vector2.hpp"
#include <SDL3/SDL_keyboard.h>
#include <mars/graphics/window.hpp>

namespace mars {
    struct input;

    struct input_events {
        void on_key_down(input& _input, char _key);
        void on_left_mouse_click(input& _input);
        void on_right_mouse_click(input& _input);
    };

    struct input : event<input_events> {
        const bool* keystate = nullptr;
        mars::vector2<size_t> mouse_position;

        inline void on_left_mouse_click() { broadcast<&input_events::on_left_mouse_click>(*this); }
        inline void on_right_mouse_click() { broadcast<&input_events::on_right_mouse_click>(*this); }

        inline void process() {
        }
    };

    namespace detail {
        void input_on_mouse_change(window& _window, const mars::vector2<size_t>& _position, const window_event_buttons& _click, input& _input) {
            _input.mouse_position = _position;
            if (_click.left_button)
                _input.on_left_mouse_click();
            else if (_click.right_button)
                _input.on_right_mouse_click();
        }
    } // namespace detail

    void input_create(input& _input, window& _window) {
        _input.keystate = SDL_GetKeyboardState(NULL);
        _window.listen<&window_event::on_mouse_change, detail::input_on_mouse_change>(_input);
    }
} // namespace mars