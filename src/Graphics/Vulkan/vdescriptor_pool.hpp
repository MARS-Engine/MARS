#ifndef __VDESCRIPTOR__POOL__
#define __VDESCRIPTOR__POOL__

#include "vtypes.hpp"

class vdevice;
class VShader;

class vdescriptor_pool {
public:
    VkDescriptorPool raw_descriptor_pool;
    vdevice* device;

    vdescriptor_pool(vdevice* _device);
    void create(VShader* _shader);
};

#endif