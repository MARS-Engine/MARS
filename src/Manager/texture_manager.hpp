#ifndef __TEXTURE__MANAGER__
#define __TEXTURE__MANAGER__

#include "Graphics/texture.hpp"

#include <vector>

using namespace std;

class texture_manager {
public:
    static vector<texture*> textures;

    static texture* get_texture(vengine* engine, const string& location);
};

#endif