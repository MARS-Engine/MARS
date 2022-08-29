#include "window.hpp"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "Input/input_handler.hpp"

void window::init(vector2 _size) {
    size = _size;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
    window_size = {static_cast<unsigned int>(size.x), static_cast<unsigned int>(size.y) };

    SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");

    raw_window = SDL_CreateWindow("Vulkan Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_size.width, window_size.height, window_flags);
}

void window::process() {
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
            case SDL_QUIT:
                _is_open = false;
                break;
            case SDL_KEYUP:
            case SDL_KEYDOWN:
                input_handler::handle_input(e.key);
                break;
        }

	}
}

void window::clean() {
	if (raw_window != nullptr)
		SDL_DestroyWindow(raw_window);
}