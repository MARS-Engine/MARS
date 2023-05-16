#ifndef MARS_WINDOW_BUILDER_
#define MARS_WINDOW_BUILDER_

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "graphics_builder.hpp"
#include <MARS/math/vector2.hpp>
namespace mars_graphics {
    class window;

    enum MARS_MOUSE_MODE {
        MARS_MOUSE_MODE_UNLOCKED,
        MARS_MOUSE_MODE_LOCKED
    };

    struct window_data {
        mars_math::vector2<int> size;
        SDL_WindowFlags flags{};
        std::string title;
        MARS_MOUSE_MODE mouse_mode;
    };

    class window_builder : graphics_builder<window> {
    private:
        window_data m_data;
    public:
        using graphics_builder::graphics_builder;

        inline window_builder& set_size(const mars_math::vector2<int>& _size) {
            m_data.size = _size;
            return *this;
        }
        inline window_builder& set_flag(SDL_WindowFlags _flag) {
            m_data.flags = _flag;
            return *this;
        }
        inline window_builder& set_title(const std::string& _title) {
            m_data.title = _title;
            return *this;
        }

        inline window_builder& enable_keyboard() {
            SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");
            return *this;
        }

        inline window_builder& set_mouse(MARS_MOUSE_MODE _mode) {
            m_data.mouse_mode = _mode;

            switch (_mode) {
                case MARS_MOUSE_MODE_UNLOCKED:
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    break;
                case MARS_MOUSE_MODE_LOCKED:
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    break;
            }
            return *this;
        }

        std::shared_ptr<window> build();
    };
}

#endif