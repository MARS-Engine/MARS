#ifndef __VTEXTURE__
#define __VTEXTURE__

#include "VTypes.hpp"
#include "Math/Vector2.hpp"
#include <string>

using namespace std;

class VBuffer;
class VDevice;
class VCommandBuffer;
class VPipeline;
class VShader;
class VDescriptorPool;

class VTexture {
public:
    VkSampler textureSampler;
    VkImageView imageView;
    VBuffer* buffer;
    VDevice* device;
    VmaAllocator allocator;
    Vector2 size;
    VkFormat format;
    
    VTexture(VDevice* device, VmaAllocator& allocator);
    void LoadTexture(string textureLocation);
    void Create(Vector2 size, VkFormat format, VkImageUsageFlagBits usage);
    void Bind(VCommandBuffer* commandBuffer, VPipeline* pipeline);
};

#endif