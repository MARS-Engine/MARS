#ifndef __INPUT__HANDLER__
#define __INPUT__HANDLER__

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <map>
#include <string>

enum INPUT_TYPE {
    DOWN,
    PRESS,
    UP
};



class input_handler {
public:
    static std::map<std::string, INPUT_TYPE> keys;

    static inline bool get_key(const std::string& key) { return keys.count(key) && keys[key] == PRESS; }
    static inline bool get_key_down(const std::string& key) { return keys.count(key) && keys[key] == DOWN; }
    static inline bool get_key_up(const std::string& key) {  return keys.count(key) && keys[key] == UP; }

    static void update();
    static void handle_input(SDL_KeyboardEvent key);
};

#endif