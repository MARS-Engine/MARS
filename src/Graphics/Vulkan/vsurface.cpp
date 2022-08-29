#include "vsurface.hpp"
#include <SDL2/SDL_vulkan.h>
#include "vinstance.hpp"
#include "../window.hpp"

vsurface::vsurface(window* _window, vinstance* _instance) {
    surf_window = _window;
    instance = _instance;
}

void vsurface::create() {
    SDL_Vulkan_CreateSurface(surf_window->raw_window, instance->raw_instance, &raw_surface);
}

void vsurface::clean() {
    vkDestroySurfaceKHR(instance->raw_instance, raw_surface, nullptr);
}