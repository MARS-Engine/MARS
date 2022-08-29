#ifndef __VDEPTH__
#define __VDEPTH__

#include "vinitializer.hpp"
#include "Math/vector2.hpp"

class vdevice;

class vdepth {
public:
    VkImageView image_view;
    VkImage image;
    VkExtent3D image_extent;
    VkFormat format;
    VmaAllocation allocation;
    VmaAllocator allocator;
    vdevice* device;
    vector2 size;

    vdepth(VmaAllocator& _allocator, vdevice* _device, vector2 _size);

    void create();
};

#endif