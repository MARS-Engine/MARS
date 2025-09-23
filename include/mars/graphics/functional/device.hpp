#pragma once

#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>
#include <mars/graphics/functional/window.hpp>

namespace mars::graphics {
    inline device device_create(instance& _instance, window& _window) {
        return _instance.engine->get_impl<device_impl>().device_create(_instance, _window);
    }

    inline void device_submit_graphics_queue(const device& _device, const sync& _sync, const device_submit_params& _submit_params, const command_buffer* _buffers, size_t _n_buffers) {
        _device.engine->get_impl<device_impl>().device_submit_graphics_queue(_device, _sync, _submit_params, _buffers, _n_buffers);
    }

    inline bool device_present(const device& _device, const sync& _sync, const swapchain& _swapchain, size_t _image_index) {
        return _device.engine->get_impl<device_impl>().device_present(_device, _sync, _swapchain, _image_index);
    }

    inline void device_wait(const device& _device) {
        _device.engine->get_impl<device_impl>().device_wait(_device);
    }

    inline void device_destroy(device& _device) {
        _device.engine->get_impl<device_impl>().device_destroy(_device);
    }
} // namespace mars::graphics