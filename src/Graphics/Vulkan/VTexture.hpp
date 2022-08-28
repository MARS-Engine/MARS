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

struct TextureImageView {
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
};

class VTexture {
private:
    void GenerateImageView(TextureImageView data);
    void GenerateSampler();
public:
    VkSampler textureSampler{};
    VkImageView imageView{};
    VBuffer* buffer = nullptr;
    VDevice* device = nullptr;
    VmaAllocator allocator;
    Vector2 size;
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    
    VTexture(VDevice* device, VmaAllocator& allocator);
    void LoadTexture(const string& textureLocation);
    void Create(Vector2 size, VkFormat format, VkImageUsageFlagBits usage);
    void Bind(VCommandBuffer* commandBuffer, VPipeline* pipeline) const;
};

#endif