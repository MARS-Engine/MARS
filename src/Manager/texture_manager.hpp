#ifndef __TEXTURE__MANAGER__
#define __TEXTURE__MANAGER__

#include "Graphics/texture.hpp"

#include <vector>



class texture_manager {
public:
    static std::vector<texture*> textures;

    static texture* get_texture(vengine* engine, const std::string& location);
};

#endif