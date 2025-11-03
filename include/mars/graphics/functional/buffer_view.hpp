#pragma once

#include <mars/graphics/backend/buffer_view.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars {
    namespace graphics {

        inline buffer_view buffer_view_create(const device& _device, const buffer_view_create_params& _params) {
            return _device.engine->get_impl<buffer_view_impl>().buffer_view_create(_device, _params);
        }

        inline void buffer_view_destroy(buffer_view& _buffer_view, const device& _device) {
            return _buffer_view.engine->get_impl<buffer_view_impl>().buffer_view_destroy(_buffer_view, _device);
        }
    } // namespace graphics
} // namespace mars