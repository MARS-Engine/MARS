#pragma once

#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>
#include <mars/graphics/backend/render_pass.hpp>

namespace mars {
    namespace graphics {

        inline render_pass render_pass_create(const device& _device, const swapchain& _swapchain) {
            return _device.engine->get_impl<render_pass_impl>().render_pass_create(_device, _swapchain);
        }

        inline void render_pass_destroy(render_pass& _render_pass, const device& _device) {
            _render_pass.engine->get_impl<render_pass_impl>().render_pass_destroy(_render_pass, _device);
        }
    } // namespace graphics
} // namespace mars