#include <mars/graphics/backend/vk/vk_window.hpp>
#include <mars/graphics/functional/window.hpp>

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <algorithm>

namespace mars::graphics::vk {
window vk_window_impl::vk_window_create(graphics_engine& _engine, const window_params& _params) {
	static bool has_init = false;
	if (!has_init) {
		has_init = true;
		if (!SDL_Init(SDL_INIT_VIDEO))
			return {};
		SDL_Vulkan_LoadLibrary(nullptr);
	}

	window result;
	result.engine = &_engine;
	result.flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
	result.sdl_window = SDL_CreateWindow(
		_params.title.c_str(),
		static_cast<int>(_params.size.x),
		static_cast<int>(_params.size.y),
		result.flags
	);

	if (!result.sdl_window)
		return {};

	int pixel_width = 0;
	int pixel_height = 0;
	SDL_GetWindowSizeInPixels(result.sdl_window, &pixel_width, &pixel_height);
	result.size = {
		static_cast<size_t>((std::max)(pixel_width, 1)),
		static_cast<size_t>((std::max)(pixel_height, 1)),
	};
	return result;
}

void vk_window_impl::vk_window_destroy(window& _window) {
	if (_window.sdl_window)
		SDL_DestroyWindow(_window.sdl_window);
	_window = {};
}
} // namespace mars::graphics::vk
