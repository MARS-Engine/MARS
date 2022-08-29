#ifndef __VDEVICE__
#define __VDEVICE__

#include "vtypes.hpp"

class vsurface;
class vinstance;

class vdevice {
public:
    VkDevice raw_device;
    VkPhysicalDevice physical_device;
    vsurface* surface;
    vinstance* instance;
    VkQueue  graphics_queue;
    uint32_t graphics_queue_family;

    vdevice(vinstance* _instance, vsurface* _surface);

    void create();
    void clean() const;
};

#endif