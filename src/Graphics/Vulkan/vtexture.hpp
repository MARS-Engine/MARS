#ifndef __VTEXTURE__
#define __VTEXTURE__

#include "vtypes.hpp"
#include "Math/vector2.hpp"
#include <string>



class vbuffer;
class vdevice;
class vcommand_buffer;
class vpipeline;
class VShader;
class vdescriptor_pool;

struct texture_image_view {
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
};

class vtexture {
private:
    void generate_image_view(texture_image_view data);
    void generate_sampler();
public:
    VkSampler sampler{};
    VkImageView image_view{};
    vbuffer* buffer = nullptr;
    vdevice* device = nullptr;
    VmaAllocator allocator;
    vector2 size;
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    
    vtexture(vdevice* _device, VmaAllocator& _allocator);
    void load_texture(const std::string& _texture_location);
    void create(vector2 _size, VkFormat _format, VkImageUsageFlagBits _usage);
    void bind(vcommand_buffer* _command_buffer, vpipeline* _pipeline) const;
};

#endif