#pragma once

#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/graphics_engine.hpp>
#include <mars/graphics/window.hpp>

namespace mars::graphics {
    inline device device_create(instance& _instance, window& _window) {
        return _instance.engine->get_impl<device_impl>().device_create(_instance, _window);
    }

    inline void device_destroy(device& _device) {
        _device.engine->get_impl<device_impl>().device_destroy(_device);
    }
} // namespace mars::graphics