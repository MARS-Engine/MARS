#include "Window.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

void Window::Init(Vector2 size) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
    windowSize = { static_cast<unsigned int>(size.x), static_cast<unsigned int>(size.y) };

	rawWindow = SDL_CreateWindow("Vulkan Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowSize.width, windowSize.height, window_flags);
}

void Window::Process() {
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT)
			isOpen = false;
	}
}

void Window::Clean() {
	if (rawWindow != nullptr)
		SDL_DestroyWindow(rawWindow);
}