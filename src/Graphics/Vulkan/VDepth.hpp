#ifndef __VDEPTH__
#define __VDEPTH__

#include "VInitializer.hpp"
#include "Math/Vector2.hpp"

class VDevice;

class VDepth {
public:
    VkImageView imageView;
    VkImage image;
    VkExtent3D imageExtent;
    VkFormat format;
    VmaAllocation allocation;
    VmaAllocator allocator;
    VDevice* device;
    Vector2 size;

    VDepth(VmaAllocator& _allocator, VDevice* device, Vector2 size);

    void Create();
};

#endif