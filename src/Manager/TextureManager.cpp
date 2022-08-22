#include "TextureManager.hpp"

vector<Texture*> TextureManager::textures;

Texture* TextureManager::GetTexture(VEngine* engine, const string& location) {
    for (auto t : textures)
        if (t->location == location && t->engine == engine)
            return t;

    auto t = new Texture(engine);
    t->LoadTexture(location);
    return t;
}