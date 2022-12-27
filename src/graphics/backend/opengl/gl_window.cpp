#include <MVRE/graphics/backend/opengl/gl_window.hpp>

using namespace mvre_graphics;

void gl_window::initialize(const std::string &_title, mvre_math::vector2<int> _size) {
    window::initialize(_title, _size);
    m_flags = SDL_WINDOW_OPENGL;
}