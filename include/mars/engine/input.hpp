#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_scancode.h"
#include "mars/math/vector2.hpp"
#include <SDL3/SDL_keyboard.h>
#include <mars/graphics/functional/window.hpp>
#include <string_view>

namespace mars {
struct input;

namespace detail {
	// NTTP wrapper — needed because const char* NTTPs don't accept string literals.
	// Copies the string into a structural type so bind<"Backspace", &fn>() compiles.
	template <size_t N>
	struct input_key {
		char data[N]{};
		constexpr input_key(const char (&s)[N]) {
			for (size_t i = 0; i < N; ++i) data[i] = s[i];
		}
		constexpr std::string_view view() const { return {data, N - 1}; }
	};

	template <size_t N> input_key(const char (&)[N]) -> input_key<N>;
}
struct input_events {
	void on_left_mouse_click(input& _input);
	void on_right_mouse_click(input& _input);
	void on_key_down(input& _input, const SDL_Scancode& _key);
};

struct input : event<input_events> {
	const bool* keystate = nullptr;
	mars::vector2<size_t> mouse_position;

	inline void on_left_mouse_click() { broadcast<&input_events::on_left_mouse_click>(*this); }
	inline void on_right_mouse_click() { broadcast<&input_events::on_right_mouse_click>(*this); }
	inline void on_key_down(const SDL_Scancode& _key) { broadcast<&input_events::on_key_down>(*this, _key); }

	// Resolves key name once on first call via SDL_GetScancodeFromName, then filters by scancode
	template <detail::input_key Name, auto F>
	static void key_thunk(input& _input, const SDL_Scancode& _key) {
		static const SDL_Scancode target = SDL_GetScancodeFromName(Name.view().data());
		if (_key == target) F(_input);
	}

	// Key names follow SDL conventions: https://wiki.libsdl.org/SDL3/SDL_Scancode
	template <detail::input_key Name, auto F>
	void bind() {
		listen<&input_events::on_key_down, key_thunk<Name, F>>();
	}
};

namespace detail {
void input_on_mouse_change(window&, const window_mouse_state& _mouse_state, input& _input) {
	_input.mouse_position = _mouse_state.position;
	if (_mouse_state.buttons.left_button_down && !_mouse_state.previous_buttons.left_button_down)
		_input.on_left_mouse_click();
	else if (_mouse_state.buttons.right_button_down && !_mouse_state.previous_buttons.right_button_down)
		_input.on_right_mouse_click();
}

void input_on_key_down(window&, const SDL_Scancode& _key, input& _input) {
	_input.on_key_down(_key);
}
} // namespace detail

void input_create(input& _input, window& _window) {
	_input.keystate = SDL_GetKeyboardState(NULL);
	_window.listen<&window_event::on_mouse_change, detail::input_on_mouse_change>(_input);
	_window.listen<&window_event::on_key_down, detail::input_on_key_down>(_input);
}
} // namespace mars
