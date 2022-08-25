#ifndef __VCOMMAND__POOL__
#define __VCOMMAND__POOL__

#include "VTypes.hpp"

class VDevice;

class VCommandPool {
public:
    VkCommandPool rawCommandPool;
    VDevice* device;

    VCommandPool(VDevice* device);

    void Create();
    void Clean();
};


#endif