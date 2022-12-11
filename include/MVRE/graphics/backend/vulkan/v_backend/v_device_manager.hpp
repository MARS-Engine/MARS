#ifndef __MVRE__V__DEVICE__MANAGER__
#define __MVRE__V__DEVICE__MANAGER__

#include "v_device.hpp"

namespace mvre_graphics {

    std::vector<v_device*> get_vulkan_devices(v_backend_instance* _backend_instance);
}

#endif