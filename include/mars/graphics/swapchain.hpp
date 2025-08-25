#pragma once

#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/graphics_engine.hpp>
#include <mars/graphics/window.hpp>

namespace mars::graphics {
    inline swapchain swapchain_create(const device& _device, const window& _window) {
        return _device.engine->get_impl<swapchain_impl>().swapchain_create(_device, _window);
    }

    inline void swapchain_destroy(swapchain& _swapchain, const device& _device) {
        _swapchain.engine->get_impl<swapchain_impl>().swapchain_destroy(_swapchain, _device);
    }
} // namespace mars::graphics