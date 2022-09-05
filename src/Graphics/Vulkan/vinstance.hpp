#ifndef __VINSTANCE__
#define __VINSTANCE__

#include "vtypes.hpp"
#include "VkBootstrap.h"

class vinstance {
public:
    static const bool enable_validation_layers;

    VkInstance raw_instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    vkb::Instance vkb_instance;

    void create();
    void clean();
};

#endif