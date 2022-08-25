#ifndef __TEXTURE__
#define __TEXTURE__

class VTexture;
class VEngine;
class CommandBuffer;
class Pipeline;

#include "Vulkan/VTypes.hpp"
#include "Math/Vector2.hpp"
#include <string>

using namespace std;

class Texture {
public:
    VTexture* vTexture;
    VEngine* engine;
    Vector2 GetSize();
    string location;

    Texture(VEngine* engine);
    void LoadTexture(string textureLocation);
    void Create(Vector2 size, VkFormat format, VkImageUsageFlagBits usage);
    void Bind(CommandBuffer* commandBuffer, Pipeline* pipeline) const;

};

#endif