#ifndef MVRE_INPUT_HANDLER_
#define MVRE_INPUT_HANDLER_

#include <pl/safe_map.hpp>
#include <SDL2/SDL.h>
#include <MVRE/math/vector2.hpp>

namespace mvre_graphics {
    class window;
}

namespace mvre_input {

    enum MVRE_INPUT_STATE {
        MVRE_INPUT_STATE_DOWN,
        MVRE_INPUT_STATE_PRESS,
        MVRE_INPUT_STATE_UP
    };

    class input {
    private:
        std::map<std::string, MVRE_INPUT_STATE> m_keys;
        mvre_math::vector2<float> m_mouse_relative;
    public:
        inline mvre_math::vector2<float> mouse_relative() { return m_mouse_relative; }
        inline void move_mouse(mvre_math::vector2<float> _value) { m_mouse_relative += _value; }

        inline bool get_key(const std::string& key) { return m_keys.count(key) && m_keys[key] == MVRE_INPUT_STATE_PRESS; }
        inline bool get_key_down(const std::string& key) { return m_keys.count(key) && m_keys[key] == MVRE_INPUT_STATE_DOWN; }
        inline bool get_key_up(const std::string& key) {  return m_keys.count(key) && m_keys[key] == MVRE_INPUT_STATE_UP; }

        void update();
        void finish_update();
        void handle_input(SDL_KeyboardEvent _key);
    };

    class input_manager {
    private:
        static pl::safe_map<mvre_graphics::window*, input*> m_window_input;
    public:
        static inline input* create_input(mvre_graphics::window* _window) {
            auto new_input = new input();
            m_window_input.lock();
            m_window_input.insert(std::make_pair(_window, new_input));
            m_window_input.unlock();
            return new_input;
        }

        static inline input* get_input(mvre_graphics::window* _window) { return m_window_input[_window]; }

        static inline void clean() {
            for (auto& pair : m_window_input)
                delete pair.second;
            m_window_input.clear();
        }
    };
}

#endif