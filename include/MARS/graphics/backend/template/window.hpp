#ifndef MARS_WINDOW_
#define MARS_WINDOW_

#include <string>
#include <MARS/debug/debug.hpp>
#include <MARS/math/vector2.hpp>
#include <MARS/math/vector3.hpp>
#include <MARS/input/input_manager.hpp>
#include "builders/window_builder.hpp"
#include "graphics_component.hpp"
#include <atomic>

namespace mars_graphics {

    /***
     * MARS window class.
     * Handles window creation
     */
    class window : public graphics_component {
    protected:
        window_data m_data;
        std::atomic<bool> m_should_close = false;

        struct SDL_Window* m_window = nullptr;

        void create() {
            m_window = SDL_CreateWindow(m_data.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_data.size.x, m_data.size.y, m_data.flags);
        }

        inline void set_data(const window_data& _data) {
            m_data = _data;
        }

        friend window_builder;
    public:
        explicit window(const std::shared_ptr<graphics_backend>& _graphics) : graphics_component(_graphics) {
            SDL_Init(SDL_INIT_VIDEO);
        }

        ~window() {
            if (m_window != nullptr)
                SDL_DestroyWindow(m_window);
        }

        [[nodiscard]] inline struct SDL_Window* raw_window() const { return m_window; }
        [[nodiscard]] inline mars_math::vector2<int> size() const { return m_data.size; }
        [[nodiscard]] inline std::atomic<bool>* should_close() { return &m_should_close; }

        virtual void process(const std::shared_ptr<mars_input::input>& _input) {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                switch (e.type) {
                    case SDL_QUIT:
                        m_should_close = true;
                        m_should_close.notify_all();
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