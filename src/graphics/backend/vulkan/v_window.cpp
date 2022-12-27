#include <MVRE/graphics/backend/vulkan/v_window.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_instance.hpp>
#include <SDL2/SDL_vulkan.h>

using namespace mvre_graphics;

void v_window::initialize(const std::string &_title, mvre_math::vector2<int> _size) {
    window::initialize(_title, _size);
    m_flags = SDL_WINDOW_VULKAN;
}

void v_window::create_surface(v_instance* _instance) {
    m_instance = _instance;

    if (!SDL_Vulkan_CreateSurface(m_window, m_instance->raw_instance(), &m_surface))
        mvre_debug::debug::error("MVRE - Vulkan - Failed to create surface");
}

void v_window::destroy_surface() {
    if (m_instance == nullptr)
        return;

    vkDestroySurfaceKHR(m_instance->raw_instance(), m_surface, nullptr);
}