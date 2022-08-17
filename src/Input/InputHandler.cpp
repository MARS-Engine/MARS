#include "InputHandler.hpp"

map<string, INPUT_TYPE> InputHandler::keys;

void InputHandler::Update() {
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

void InputHandler::HandleInput(SDL_KeyboardEvent key) {
    string name = SDL_GetKeyName(key.keysym.sym);

    if (key.type == SDL_KEYUP)
        keys[name] = UP;
    else
        keys.insert(pair<string, INPUT_TYPE>(name, DOWN));
}