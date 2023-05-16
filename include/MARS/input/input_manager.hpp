#ifndef MARS_INPUT_HANDLER_
#define MARS_INPUT_HANDLER_

#include <pl/safe_map.hpp>
#include <SDL2/SDL.h>
#include <MARS/math/vector2.hpp>
#include <MARS/engine/singleton.hpp>

namespace mars_graphics {
    class window;
}

namespace mars_input {

    enum MARS_INPUT_STATE {
        MARS_INPUT_STATE_DOWN,
        MARS_INPUT_STATE_PRESS,
        MARS_INPUT_STATE_UP
    };

    class input : public mars_engine::singleton {
    private:
        std::map<std::string, MARS_INPUT_STATE> m_keys;
        mars_math::vector2<float> m_mouse_relative;
    public:
        inline mars_math::vector2<float> mouse_relative() { return m_mouse_relative; }
        inline void move_mouse(const mars_math::vector2<float>& _value) { m_mouse_relative += _value; }

        inline bool get_key(const std::string& key) { return m_keys.count(key) && m_keys[key] == MARS_INPUT_STATE_PRESS; }
        inline bool get_key_down(const std::string& key) { return m_keys.count(key) && m_keys[key] == MARS_INPUT_STATE_DOWN; }
        inline bool get_key_up(const std::string& key) {  return m_keys.count(key) && m_keys[key] == MARS_INPUT_STATE_UP; }

        void update();
        void finish_update();
        void handle_input(SDL_KeyboardEvent _key);
    };
}

#endif