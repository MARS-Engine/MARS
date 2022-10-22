#include "MVRE/graphics/window.hpp"

using namespace mvre_graphics;
using namespace mvre_math;

void window::init(const std::string _title, vector2<int> _size, MVRE_INSTANCE_TYPE _type) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");

    m_size = _size;
    m_title = _title;
    m_flags = _type == MVRE_INSTANCE_TYPE_VULKAN ? SDL_WINDOW_VULKAN : SDL_WINDOW_OPENGL;
}

void window::create() {
    raw_window = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_size.x(), m_size.y(), m_flags);
}

void window::process() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
            case SDL_QUIT:
                m_should_close = true;
                break;
        }
    }
}

void window::clean() {
    if (raw_window != nullptr)
        SDL_DestroyWindow(raw_window);
}