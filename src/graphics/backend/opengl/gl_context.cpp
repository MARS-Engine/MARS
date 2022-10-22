#include "MVRE/graphics/backend/opengl/gl_context.hpp"

using namespace mvre_graphics_opengl;

void gl_context::create() {
    raw_context = SDL_GL_CreateContext(surf_window.raw_window);
}

void gl_context::clean() {
    SDL_GL_DeleteContext(raw_context);
}