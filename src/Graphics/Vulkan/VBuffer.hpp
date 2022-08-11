#ifndef __VBUFFER__
#define __VBUFFER__

#include "VTypes.hpp"
#include "Math/Vector2.hpp"

class VDevice;
class VInstance;
class VCommandBuffer;

enum VBufferBindType {
    VBUFFER_VERTEX_BIND,
    VBUFFER_INDEX_BIND
};


class VBuffer {
public:
    VkDescriptorSet descriptor;
    VkBuffer buffer;
    VmaAllocation allocation;
    VkImage image;
    VmaAllocation imageAllocation;
    VmaAllocator allocator;
    size_t bufferSize;
    VDevice* device;
    VkExtent3D imageExtent;
    VBufferBindType type;

    VBuffer(VDevice* device, VmaAllocator& allocator);

    static VmaAllocator GenerateAllocator(VDevice* device, VInstance* instance);

    void Create(size_t size, int usage, VmaMemoryUsage memoryUsage);
    void CreateImage(Vector2 size, VkFormat format, VkImageUsageFlags usage, VkExtent3D extent);
    void CopyBuffer(VBuffer* src) const;
    void CopyBufferImage() const;
    void Update(void* data) const;
    void TransitionImageLayout(bool begin);
    void Bind(VCommandBuffer* commandBuffer, VBufferBindType type);
};

#endif