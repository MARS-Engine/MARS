#include <MVRE/graphics/backend/vulkan/v_window.hpp>

using namespace mvre_graphics;

void v_window::initialize(const std::string &_title, mvre_math::vector2<int> _size) {
    window::initialize(_title, _size);
    m_flags = SDL_WINDOW_VULKAN;
}