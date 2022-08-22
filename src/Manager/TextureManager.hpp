#ifndef __TEXTUREMANAGER__
#define __TEXTUREMANAGER__

#include "Graphics/Texture.hpp"

#include <vector>

using namespace std;

class TextureManager {
public:
    static vector<Texture*> textures;

    static Texture* GetTexture(VEngine* engine, const string& location);
};

#endif