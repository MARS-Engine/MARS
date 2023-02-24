#ifndef MARS_V_DEVICE_MANAGER_
#define MARS_V_DEVICE_MANAGER_

#include "v_device.hpp"

namespace mars_graphics {

    std::vector<v_device*> get_vulkan_devices(vulkan_backend* _backend_instance);
}

#endif