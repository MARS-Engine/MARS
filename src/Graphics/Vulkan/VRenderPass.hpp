#ifndef __VRENDERPASS__
#define __VRENDERPASS__

#include "VTypes.hpp"
#include "Math/Vector2.hpp"

class VDevice;
class VDepth;

class VRenderPass {
public:
    VkRenderPass rawRenderPass;
    VDevice* device;
    VDepth* depth;
    VmaAllocator allocator;

    VRenderPass(VmaAllocator& allocator, VDevice* device);

    void Create(Vector2 size, VkFormat format);
    void Clean() const;
};

#endif