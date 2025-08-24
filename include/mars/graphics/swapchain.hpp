#pragma once

#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/graphics_engine.hpp>
#include <mars/graphics/window.hpp>

namespace mars::graphics {
    inline swapchain swapchain_create(instance& _instance, window& _window) {
        return _instance.engine->get_impl<swapchain_impl>().swapchain_create(_instance, _window);
    }

    inline void swapchain_destroy(swapchain& _swapchain, instance& _instance) {
        _instance.engine->get_impl<swapchain_impl>().swapchain_destroy(_swapchain, _instance);
    }
} // namespace mars::graphics