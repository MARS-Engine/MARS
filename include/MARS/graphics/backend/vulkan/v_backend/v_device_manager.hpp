#ifndef MARS_V_DEVICE_MANAGER_
#define MARS_V_DEVICE_MANAGER_

#include "v_device.hpp"

namespace mars_graphics {

    std::vector<v_device*> get_vulkan_devices(v_backend_instance* _backend_instance);
}

#endif