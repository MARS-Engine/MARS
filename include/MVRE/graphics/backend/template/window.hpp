#ifndef MVRE_WINDOW_
#define MVRE_WINDOW_

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <string>
#include <MVRE/debug/debug.hpp>
#include "MVRE/math/vector2.hpp"

namespace mvre_graphics {

    /***
     * MVRE window class.
     * Handles window creation
     */
    class window {
    protected:
        mvre_math::vector2<int> m_size;
        bool m_should_close = false;
        SDL_WindowFlags m_flags{};
        std::string m_title;

        struct SDL_Window* m_window = nullptr;
    public:

        ~window() {
            if (m_window != nullptr)
                SDL_DestroyWindow(m_window);
        }

        inline struct SDL_Window* raw_window() { return m_window; }

        /**
         * Get size of window
         * @return size inside a vector2 of type int
         */
        inline mvre_math::vector2<int> size() const { return m_size; }

        /**
         * Check if window received close flag
         * @return true if should close
         */
        inline bool should_close() const { return m_should_close; }

        /**
         * Initialize windows values and prepare for creation
         * @param _title window title
         * @param _size window size
         */
        virtual void initialize(const std::string& _title, mvre_math::vector2<int> _size) {
            SDL_Init(SDL_INIT_VIDEO);
            SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");

            m_size = _size;
            m_title = _title;
        }

        /**
         * Create the window
         */
        virtual void create() {
            m_window = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_size.x(), m_size.y(), m_flags);
        }

        /**
         * Process Window PoolEvents
         */
        virtual void process() {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                switch (e.type) {
                    case SDL_QUIT:
                        m_should_close = true;
                        break;
                }
            }
        }
    };
}
#endif