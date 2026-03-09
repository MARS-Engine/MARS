#include "SDL3/SDL_video.h"
#include <mars/graphics/backend/dx12/dx_window.hpp>

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <windows.h>

#include <algorithm>
#include <unordered_map>

namespace mars::graphics::dx {
namespace __internal {
std::unordered_map<SDL_Window*, HWND> window_handle_map;
}

window dx_window_impl::dx_window_create(graphics_engine& _engine, const window_params& _params) {
	static bool has_init = false;

	if (!has_init) {
		has_init = true;
		if (!SDL_Init(SDL_INIT_VIDEO))

			return {};
	}

	window result;
	result.engine = &_engine;
	result.sdl_window = SDL_CreateWindow(_params.title.c_str(), _params.size.x, _params.size.y, SDL_WINDOW_RESIZABLE);

	HWND g_main_hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(result.sdl_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

	if (!g_main_hwnd) {
		SDL_DestroyWindow(result.sdl_window);
		return {};
	}

	int pixel_width = 0;
	int pixel_height = 0;
	SDL_GetWindowSizeInPixels(result.sdl_window, &pixel_width, &pixel_height);
	result.size = {
	    static_cast<size_t>((std::max)(pixel_width, 1)),
	    static_cast<size_t>((std::max)(pixel_height, 1)),
	};

	__internal::window_handle_map[result.sdl_window] = g_main_hwnd;
	return result;
}

void dx_window_impl::dx_window_destroy(window& _window) {
	if (!_window.sdl_window)
		return;

	__internal::window_handle_map.erase(_window.sdl_window);
	SDL_DestroyWindow(_window.sdl_window);
	if (__internal::window_handle_map.size() == 0)
		SDL_Quit();
	_window = {};
}
} // namespace mars::graphics::dx
