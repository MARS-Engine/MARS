#ifndef __VDEVICE__
#define __VDEVICE__

#include "VTypes.hpp"

class VSurface;
class VInstance;

class VDevice {
public:
    VkDevice rawDevice;
    VkPhysicalDevice physicalDevice;
    VSurface* surface;
    VInstance* instance;
    VkQueue  graphicsQueue;
    uint32_t graphicsQueueFamily;

    VDevice(VInstance* instance, VSurface* surface);

    void Create();
    void Clean() const;
};

#endif