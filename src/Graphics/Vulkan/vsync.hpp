#ifndef __VSYNC__
#define __VSYNC__

#include "vtypes.hpp"
#include <vector>

class vdevice;

class vsync {
public:
    std::vector<VkFence> fences;
    std::vector<VkSemaphore> presents;
    std::vector<VkSemaphore> renders;
    vdevice* device;

    vsync(vdevice* _device);
    void create();
    void wait(size_t _frame);
};

#endif