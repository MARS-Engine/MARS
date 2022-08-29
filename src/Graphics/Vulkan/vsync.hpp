#ifndef __VSYNC__
#define __VSYNC__

#include "vtypes.hpp"
#include <vector>

class vdevice;

class vsync {
public:
    vector<VkFence> fences;
    vector<VkSemaphore> presents;
    vector<VkSemaphore> renders;
    vdevice* device;

    vsync(vdevice* _device);
    void create();
    void wait(size_t _frame);
};

#endif