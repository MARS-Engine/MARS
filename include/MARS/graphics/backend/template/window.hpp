#ifndef MARS_WINDOW_
#define MARS_WINDOW_

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <string>
#include <MARS/debug/debug.hpp>
#include <MARS/math/vector2.hpp>
#include <MARS/math/vector3.hpp>
#include <MARS/input/input_manager.hpp>

namespace mars_graphics {

    /***
     * MARS window class.
     * Handles window creation
     */
    class window {
    protected:
        mars_math::vector2<size_t> m_size;
        bool m_should_close = false;
        SDL_WindowFlags m_flags{};
        std::string m_title;

        struct SDL_Window* m_window = nullptr;
    public:

        inline struct SDL_Window* raw_window() { return m_window; }

        inline mars_math::vector2<size_t> size() const { return m_size; }

        inline bool should_close() const { return m_should_close; }

        virtual void initialize(const std::string& _title, mars_math::vector2<size_t> _size) {
            SDL_Init(SDL_INIT_VIDEO);
            SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");
            SDL_SetRelativeMouseMode(SDL_TRUE);

            m_size = _size;
            m_title = _title;
        }

        virtual void create() {
            m_window = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_size.x(), m_size.y(), m_flags);
        }

        virtual void create_surface() { }

        virtual void destroy() {
            if (m_window != nullptr)
                SDL_DestroyWindow(m_window);
        }

        virtual void process(mars_input::input* _input) {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                switch (e.type) {
                    case SDL_QUIT:
                        m_should_close = true;
                        break;
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                        _input->handle_input(e.key);
                        break;
                    case SDL_MOUSEMOTION:
                        _input->move_mouse({ (float)e.motion.xrel, (float)e.motion.yrel });
                        break;
                }
            }
        }
    };
}
#endif