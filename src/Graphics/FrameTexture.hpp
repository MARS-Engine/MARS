#ifndef __FRAME__TEXTURE__
#define __FRAME__TEXTURE__

#include "Texture.hpp"
#include <vector>

using namespace std;

class FrameTexture {
public:
    vector<Texture*> textures;
    Vector2 size;
    VEngine* engine;

    FrameTexture(VEngine* engine);

    void Create(size_t nFrames, Vector2 frameSize, VkFormat format, VkImageUsageFlagBits usage);
};

#endif