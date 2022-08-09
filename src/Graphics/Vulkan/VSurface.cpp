#include "VSurface.hpp"
#include <SDL2/SDL_vulkan.h>
#include "VInstance.hpp"
#include "../Window.hpp"

VSurface::VSurface(Window* _window, VInstance* _instance) {
    window = _window;
    instance = _instance;
}

void VSurface::Create() {
    SDL_Vulkan_CreateSurface(window->rawWindow, instance->rawInstance, &rawSurface);
}

void VSurface::Clean() {
    vkDestroySurfaceKHR(instance->rawInstance, rawSurface, nullptr);
}