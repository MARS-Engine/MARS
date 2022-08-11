#ifndef __VDESCRIPTORPOOL__
#define __VDESCRIPTORPOOL__

#include "VTypes.hpp"

class VDevice;
class VShader;

class VDescriptorPool {
public:
    VkDescriptorPool rawDescriptorPool;
    VDevice* device;

    VDescriptorPool(VDevice* device);
    void Create(VShader* shader);
};

#endif