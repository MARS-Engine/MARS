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
	bool left_button;
	bool right_button;
};

struct window_event {
	void on_mouse_change(window&, const mars::vector2<size_t>& _position, const window_event_buttons& _click);
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

	inline void resize(const mars::vector2<size_t>& _size) {
		size = _size;
		broadcast<&window_event::on_resize>(*this, _size);
	}

	inline void mouse_change(const mars::vector2<size_t>& _position, const window_event_buttons& _click) {
		broadcast<&window_event::on_mouse_change>(*this, _position, _click);
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
		} else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
			window* target_window = find_window(event.button.windowID);
			if (target_window == nullptr)
				continue;
			target_window->mouse_change(
				{static_cast<size_t>(event.button.x), static_cast<size_t>(event.button.y)},
				{event.button.button == SDL_BUTTON_LEFT, event.button.button == SDL_BUTTON_RIGHT}
			);
		} else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
			window* target_window = find_window(event.wheel.windowID);
			if (target_window != nullptr)
				target_window->wheel_change = event.wheel.y - event.wheel.x;
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
