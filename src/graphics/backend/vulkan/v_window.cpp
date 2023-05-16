#include <MARS/graphics/backend/vulkan/v_window.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_instance.hpp>
#include <SDL2/SDL_vulkan.h>

using namespace mars_graphics;

void v_window::create_surface(v_instance* _instance) {
    m_instance = _instance;

    if (!SDL_Vulkan_CreateSurface(m_window, m_instance->raw_instance(), &m_surface))
        mars_debug::debug::error("MARS - Vulkan - Failed to create surface");
}

void v_window::destroy_surface() {
    if (m_instance == nullptr)
        return;

    vkDestroySurfaceKHR(m_instance->raw_instance(), m_surface, nullptr);
}