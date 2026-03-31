#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <cstddef>

#include <mars/event/event.hpp>
#include <mars/graphics/backend/window.hpp>
#include <mars/math/vector2.hpp>
#include <mars/meta/type_erased.hpp>
#include <span>
#include <vector>

namespace mars {
struct graphics_engine;
struct window;

struct window_event_buttons {
	bool left_button_down = false;
	bool right_button_down = false;
	bool middle_button_down = false;
};

struct window_mouse_state {
	mars::vector2<size_t> position = {};
	mars::vector2<size_t> previous_position = {};
	window_event_buttons buttons = {};
	window_event_buttons previous_buttons = {};
};

struct window_mouse_wheel_state {
	float delta = 0.0f;
	mars::vector2<size_t> position = {};
};

struct window_event {
	void on_mouse_change(window&, const window_mouse_state& _mouse_state);
	void on_mouse_motion(window&, const window_mouse_state& _mouse_state);
	void on_mouse_wheel(window&, const window_mouse_wheel_state& _mouse_wheel_state);
	void on_resize(window&, const mars::vector2<size_t>& _size);
	void on_key_down(window&, const SDL_Scancode& _key);
	void on_focus_changed(window&, const bool& _focused);
	void on_close(window&);
};

struct window : event<window_event> {
	graphics_engine* engine;
	meta::type_erased_ptr data;
	mars::vector2<size_t> size;
	SDL_Window* sdl_window = nullptr;
	Uint64 now = 0;
	Uint64 last = 0;
	float delta_time = 0.0f;
	SDL_WindowFlags flags = 0;

	float wheel_change = 0.0f;
	window_mouse_state mouse_state = {};

	inline void resize(const mars::vector2<size_t>& _size) {
		size = _size;
		broadcast<&window_event::on_resize>(*this, _size);
	}

	inline void mouse_change(const mars::vector2<size_t>& _position, const window_event_buttons& _buttons) {
		mouse_state.previous_position = mouse_state.position;
		mouse_state.previous_buttons = mouse_state.buttons;
		mouse_state.position = _position;
		mouse_state.buttons = _buttons;
		broadcast<&window_event::on_mouse_change>(*this, mouse_state);
	}

	inline void mouse_motion(const mars::vector2<size_t>& _position) {
		mouse_state.previous_position = mouse_state.position;
		mouse_state.previous_buttons = mouse_state.buttons;
		mouse_state.position = _position;
		broadcast<&window_event::on_mouse_motion>(*this, mouse_state);
	}

	inline void mouse_wheel(const window_mouse_wheel_state& _mouse_wheel_state) {
		wheel_change = _mouse_wheel_state.delta;
		broadcast<&window_event::on_mouse_wheel>(*this, _mouse_wheel_state);
	}

	inline void close() {
		broadcast<&window_event::on_close>(*this);
	}

	inline void key_down(const SDL_Scancode& _key) {
		broadcast<&window_event::on_key_down>(*this, _key);
	}

	inline void focus_changed(const bool& _focused) {
		broadcast<&window_event::on_focus_changed>(*this, _focused);
	}
};

namespace graphics {
window window_create(graphics_engine& _engine, const window_params& _params);
void window_destroy(window& _window);
} // namespace graphics

template <typename EventHook>
inline void window_process_events(std::span<window*> _windows, EventHook&& _event_hook) {
	const Uint64 now = SDL_GetPerformanceCounter();
	const float frequency = static_cast<float>(SDL_GetPerformanceFrequency());
	for (window* window_ptr : _windows) {
		if (window_ptr == nullptr)
			continue;
		window_ptr->now = now;
		window_ptr->delta_time = (window_ptr->now - window_ptr->last) / frequency;
		window_ptr->last = window_ptr->now;
		window_ptr->wheel_change = 0.0f;
	}

	const auto find_window = [&](Uint32 _window_id) -> window* {
		for (window* window_ptr : _windows) {
			if (window_ptr == nullptr || window_ptr->sdl_window == nullptr)
				continue;
			if (SDL_GetWindowID(window_ptr->sdl_window) == _window_id)
				return window_ptr;
		}
		return nullptr;
	};

	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		_event_hook(event);

		if (event.type == SDL_EVENT_WINDOW_RESIZED ||
			event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
			window* target_window = find_window(event.window.windowID);
			if (target_window == nullptr)
				continue;
			int pixel_width = 0;
			int pixel_height = 0;
			SDL_GetWindowSizeInPixels(target_window->sdl_window, &pixel_width, &pixel_height);
			if (pixel_width > 0 && pixel_height > 0)
				target_window->resize({static_cast<size_t>(pixel_width), static_cast<size_t>(pixel_height)});
		} 
		else if (event.type == SDL_EVENT_MOUSE_MOTION) {
			window* target_window = find_window(event.motion.windowID);
			if (target_window == nullptr)
				continue;
			target_window->mouse_motion({static_cast<size_t>(event.motion.x), static_cast<size_t>(event.motion.y)});
		}
		else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
			window* target_window = find_window(event.button.windowID);
			if (target_window == nullptr)
				continue;
			window_event_buttons button_state = target_window->mouse_state.buttons;
			if (event.button.button == SDL_BUTTON_LEFT)
				button_state.left_button_down = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
			else if (event.button.button == SDL_BUTTON_RIGHT)
				button_state.right_button_down = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
			else if (event.button.button == SDL_BUTTON_MIDDLE)
				button_state.middle_button_down = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
			target_window->mouse_change(
				{static_cast<size_t>(event.button.x), static_cast<size_t>(event.button.y)},
				button_state
			);
		} 
		else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
			window* target_window = find_window(event.wheel.windowID);
			if (target_window != nullptr)
				target_window->mouse_wheel({
					.delta = event.wheel.y,
					.position = {
						static_cast<size_t>(event.wheel.mouse_x < 0.0f ? 0.0f : event.wheel.mouse_x),
						static_cast<size_t>(event.wheel.mouse_y < 0.0f ? 0.0f : event.wheel.mouse_y)
					}
				});
		} else if (event.type == SDL_EVENT_KEY_DOWN) {
			window* target_window = find_window(event.key.windowID);
			if (target_window != nullptr)
				target_window->key_down(event.key.scancode);
		} else if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED || event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
			window* target_window = find_window(event.window.windowID);
			if (target_window != nullptr)
				target_window->focus_changed(event.type == SDL_EVENT_WINDOW_FOCUS_GAINED);
		} else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
			window* target_window = find_window(event.window.windowID);
			if (target_window != nullptr)
				target_window->close();
		} else if (event.type == SDL_EVENT_QUIT) {
			for (window* window_ptr : _windows) {
				if (window_ptr != nullptr)
					window_ptr->close();
			}
		}
	}
}

template <typename EventHook>
inline void window_process_events(window& _window, EventHook&& _event_hook) {
	window* windows[] = { &_window };
	window_process_events(std::span<window*>(windows), std::forward<EventHook>(_event_hook));
}

inline void window_process_events(window& _window) {
	window_process_events(_window, [](const SDL_Event&) {});
}
} // namespace mars
