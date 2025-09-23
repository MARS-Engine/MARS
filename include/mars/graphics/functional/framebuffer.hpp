#pragma once

#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/framebuffer.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars {
    namespace graphics {

        inline framebuffer framebuffer_create(const device& _device, const swapchain& _swapchain, const render_pass& _render_pass) {
            return _device.engine->get_impl<framebuffer_impl>().framebuffer_create(_device, _swapchain, _render_pass);
        }

        inline void framebuffer_destroy(framebuffer& _framebuffer, const device& _device) {
            _framebuffer.engine->get_impl<framebuffer_impl>().framebuffer_destroy(_framebuffer, _device);
        }
    } // namespace graphics
} // namespace mars