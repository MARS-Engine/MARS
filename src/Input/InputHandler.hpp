#ifndef __INPUTHANDLER__
#define __INPUTHANDLER__

#include <SDL2/SDL.h>
#include <map>
#include <string>

enum INPUT_TYPE {
    DOWN,
    PRESS,
    UP
};

using namespace std;

class InputHandler {
public:
    static map<string, INPUT_TYPE> keys;

    static inline bool GetKey(const string& key) { return keys.count(key) && keys[key] == PRESS; }
    static inline bool GetKeyDown(const string& key) { return keys.count(key) && keys[key] == DOWN; }
    static inline bool GetKeyUp(const string& key) {  return keys.count(key) && keys[key] == UP; }

    static void Update();
    static void HandleInput(SDL_KeyboardEvent key);
};

#endif