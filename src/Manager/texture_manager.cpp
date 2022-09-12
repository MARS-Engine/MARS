#include "texture_manager.hpp"

std::vector<texture*> texture_manager::textures;

texture* texture_manager::get_texture(vengine* engine, const std::string& location) {
    for (auto t : textures)
        if (t->location == location && t->engine == engine)
            return t;

    auto t = new texture(engine);
    t->load_texture(location);
    return t;
}