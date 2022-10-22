#ifndef __MVRE__WINDOW__
#define __MVRE__WINDOW__

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <string>

#include "MVRE/math/vector2.hpp"

#include "mvre_graphics_types.hpp"

namespace mvre_graphics {

    class window {
    private:
        mvre_math::vector2<int> m_size;
        bool m_should_close = false;
        SDL_WindowFlags m_flags;
        std::string m_title;
    public:
        struct SDL_Window* raw_window = nullptr;

        inline mvre_math::vector2<int> get_size() { return m_size; }

        inline bool should_close() { return m_should_close; }

        void init(const std::string _title, mvre_math::vector2<int> _size, MVRE_INSTANCE_TYPE _type);
        void create();
        void process();
        void clean();
    };
}

#endif