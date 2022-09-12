#include "input_handler.hpp"

std::map<std::string, INPUT_TYPE> input_handler::keys;

void input_handler::update() {
    for (auto pair = keys.cbegin(); pair != keys.cend();) {
        switch (pair->second) {
            case UP:
                pair = keys.erase(pair);
                break;
            case DOWN:
                keys[pair->first] = PRESS;
                ++pair;
                break;
            case PRESS:
                ++pair;
                break;
        }
    }
}

void input_handler::handle_input(SDL_KeyboardEvent key) {
    std::string name = SDL_GetKeyName(key.keysym.sym);

    if (key.type == SDL_KEYUP)
        keys[name] = UP;
    else
        keys.insert(std::pair<std::string, INPUT_TYPE>(name, DOWN));
}