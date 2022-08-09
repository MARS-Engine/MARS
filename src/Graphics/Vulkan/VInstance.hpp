#ifndef __VINSTANCE__
#define __VINSTANCE__

#include "VTypes.hpp"
#include "VkBootstrap.h"

class VInstance {
public:
    static const bool enableValidationLayers;

    VkInstance rawInstance;
    VkDebugUtilsMessengerEXT debugMessenger;
    vkb::Instance vkbInstance;

    void Create();
    void Clean();
};

#endif