#ifndef __VBUFFER__
#define __VBUFFER__

#include "vtypes.hpp"
#include "Math/vector2.hpp"

class vdevice;
class vinstance;
class vcommand_buffer;

enum vbuffer_bind_type {
    VBUFFER_VERTEX_BIND,
    VBUFFER_INDEX_BIND
};


class vbuffer {
public:
    VkDescriptorSet descriptor;
    VkBuffer buffer;
    VmaAllocation allocation;
    VkImage image;
    VmaAllocation image_allocation;
    VmaAllocator allocator;
    size_t buffer_size;
    vdevice* device;
    VkExtent3D image_extent;
    vbuffer_bind_type type;

    vbuffer(vdevice* _device, VmaAllocator& _allocator);

    static VmaAllocator GenerateAllocator(vdevice* _device, vinstance* _instance);

    void Create(size_t _size, int _usage, VmaMemoryUsage _memory_usage);
    void CreateImage(VkFormat _format, VkImageUsageFlags _usage, VkExtent3D _extent);
    void CopyBuffer(vbuffer* _src) const;
    void CopyBufferImage() const;
    void Update(void* _data) const;
    void TransitionImageLayout(bool _begin);
    void Bind(vcommand_buffer* _command_buffer, vbuffer_bind_type _type);
};

#endif