#ifndef __QUICK__SUBMIT__
#define __QUICK__SUBMIT__

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