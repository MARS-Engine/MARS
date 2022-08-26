#include "Window.hpp"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "Input/InputHandler.hpp"

void Window::Init(Vector2 _size) {
    size = _size;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
    windowSize = { static_cast<unsigned int>(size.x), static_cast<unsigned int>(size.y) };

    SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");

	rawWindow = SDL_CreateWindow("Vulkan Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowSize.width, windowSize.height, window_flags);
}

void Window::Process() {
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
            case SDL_QUIT:
                isOpen = false;
                break;
            case SDL_KEYUP:
            case SDL_KEYDOWN:
                InputHandler::HandleInput(e.key);
                break;
        }

	}
}

void Window::Clean() {
	if (rawWindow != nullptr)
		SDL_DestroyWindow(rawWindow);
}