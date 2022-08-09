#ifndef __VCOMMANDPOOL__
#define __VCOMMANDPOOL__

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