#ifndef __TEXTURE__
#define __TEXTURE__

class VTexture;
class VEngine;
class CommandBuffer;
class Pipeline;

#include "Math/Vector2.hpp"
#include <string>

using namespace std;

class Texture {
public:
    VTexture* vTexture;
    VEngine* engine;
    Vector2 GetSize();

    Texture(VEngine* engine);
    void LoadTexture(string textureLocation);
    void Bind(CommandBuffer* commandBuffer, Pipeline* pipeline);

};

#endif