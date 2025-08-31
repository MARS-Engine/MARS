#pragma once

#include <mars/graphics/backend/sync.hpp>
#include <mars/graphics/graphics_engine.hpp>

namespace mars::graphics {
    inline sync sync_create(const device& _device, size_t _size) {
        return _device.engine->get_impl<sync_impl>().sync_create(_device, _size);
    }

    inline void sync_wait(const sync& _sync, const device& _device, size_t _current_frame) {
        _sync.engine->get_impl<sync_impl>().sync_wait(_sync, _device, _current_frame);
    }

    inline void sync_reset(const sync& _sync, const device& _device, size_t _current_frame) {
        _sync.engine->get_impl<sync_impl>().sync_reset(_sync, _device, _current_frame);
    }

    inline bool sync_get_next_image(const sync& _sync, const device& _device, const swapchain& _swapchain, size_t _current_frame, size_t& _image_index) {
        return _sync.engine->get_impl<sync_impl>().sync_get_next_image(_sync, _device, _swapchain, _current_frame, _image_index);
    }

    inline void sync_destroy(sync& _sync, const device& _device) {
        _sync.engine->get_impl<sync_impl>().sync_destroy(_sync, _device);
    }
} // namespace mars::graphics