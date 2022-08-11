#ifndef __QUICKSUBMIT__
#define __QUICKSUBMIT__

#include "VTypes.hpp"

class VCommandBuffer;
class VCommandPool;
class VDevice;

class QuickSubmit {
public:
    VkFence rawFence;
    VCommandPool* commandPool;
    VCommandBuffer* commandBuffer;
    VDevice* device;

    QuickSubmit(VDevice* device);

    void Begin();
    void End();
    void Clean();
};

#endif