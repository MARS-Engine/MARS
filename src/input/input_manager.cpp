#include <MVRE/input/input_manager.hpp>

using namespace mvre_input;


pl::safe_map<mvre_graphics::window*, input*> input_manager::m_window_input;

void input::update() {
    for (auto pair = m_keys.cbegin(); pair != m_keys.cend();) {
        switch (pair->second) {
            case MVRE_INPUT_STATE_UP:
                pair = m_keys.erase(pair);
                break;
            case MVRE_INPUT_STATE_DOWN:
                m_keys[pair->first] = MVRE_INPUT_STATE_PRESS;
                ++pair;
                break;
            case MVRE_INPUT_STATE_PRESS:
                ++pair;
                break;
        }
    }
}

void input::finish_update() {
    m_mouse_relative = { 0, 0 };
}

void input::handle_input(SDL_KeyboardEvent _key) {
    auto name = SDL_GetKeyName(_key.keysym.sym);

    if (_key.type == SDL_KEYUP)
        m_keys[name] = MVRE_INPUT_STATE_UP;
    else
        m_keys.insert(std::pair<std::string, MVRE_INPUT_STATE>(name, MVRE_INPUT_STATE_DOWN));
}