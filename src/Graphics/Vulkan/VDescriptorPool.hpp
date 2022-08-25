#ifndef __VDESCRIPTOR__POOL__
#define __VDESCRIPTOR__POOL__

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