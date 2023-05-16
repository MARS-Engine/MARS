#include <MARS/input/input_manager.hpp>

using namespace mars_input;

void input::update() {
    for (auto pair = m_keys.cbegin(); pair != m_keys.cend();) {
        switch (pair->second) {
            case MARS_INPUT_STATE_UP:
                pair = m_keys.erase(pair);
                break;
            case MARS_INPUT_STATE_DOWN:
                m_keys[pair->first] = MARS_INPUT_STATE_PRESS;
                ++pair;
                break;
            case MARS_INPUT_STATE_PRESS:
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
        m_keys[name] = MARS_INPUT_STATE_UP;
    else
        m_keys.insert(std::pair<std::string, MARS_INPUT_STATE>(name, MARS_INPUT_STATE_DOWN));
}