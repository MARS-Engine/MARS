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
};

namespace graphics {
window window_create(graphics_engine& _engine, const window_params& _params);
void window_destroy(window& _window);
} // namespace graphics

template <typename EventHook>
inline void window_process_events(window& _window, EventHook&& _event_hook) {
	_window.now = SDL_GetPerformanceCounter();
	_window.delta_time = (_window.now - _window.last) / static_cast<float>(SDL_GetPerformanceFrequency());
	_window.last = _window.now;

	SDL_Event event;

	_window.wheel_change = 0.0f;

	while (SDL_PollEvent(&event)) {
		_event_hook(event);

		if (event.type == SDL_EVENT_WINDOW_RESIZED ||
		    event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
			SDL_Window* sdl_win = SDL_GetWindowFromID(event.window.windowID);
			if (sdl_win == _window.sdl_window) {
				int pixel_width = 0;
				int pixel_height = 0;
				SDL_GetWindowSizeInPixels(sdl_win, &pixel_width, &pixel_height);
				if (pixel_width > 0 && pixel_height > 0)
					_window.resize({static_cast<size_t>(pixel_width), static_cast<size_t>(pixel_height)});
			}
		} else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
			_window.mouse_change({(size_t)event.motion.x, (size_t)event.motion.y}, {event.button.button == SDL_BUTTON_LEFT, event.button.button == SDL_BUTTON_RIGHT});
		} else if (event.type == SDL_EVENT_QUIT)
			_window.close();
		else if (event.type == SDL_EVENT_MOUSE_WHEEL)
			_window.wheel_change = event.wheel.y - event.wheel.x;
	}
}

inline void window_process_events(window& _window) {
	window_process_events(_window, [](const SDL_Event&) {});
}
} // namespace mars
