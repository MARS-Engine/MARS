#ifndef __VSYNC__
#define __VSYNC__

#include "VTypes.hpp"
#include <vector>

class VDevice;

class VSync {
public:
    vector<VkFence> fences;
    vector<VkSemaphore> presents;
    vector<VkSemaphore> renders;
    VDevice* device;

    VSync(VDevice* device);
    void Create();
    void Wait(size_t frame);
};

#endif