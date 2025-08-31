#pragma once

#include <mars/math/vector4.hpp>
#include <mars/meta/type_erasure.hpp>

#include <cstddef>

namespace mars {
    struct device;
    struct swapchain;
    struct command_buffer;
    struct framebuffer;
    struct graphics_backend_functions;

    struct render_pass {
        graphics_backend_functions* engine;
        meta::type_erasure_ptr data;
    };

    struct render_pass_bind_param {
        size_t image_index;
        vector4<float> clear_color;
    };

    struct render_pass_impl {
        render_pass (*render_pass_create)(const device& _device, const swapchain& _swapchain) = nullptr;
        void (*render_pass_bind)(const render_pass& _render_pass, const command_buffer& _command_buffer, const framebuffer& _framebuffer, const render_pass_bind_param& _params) = nullptr;
        void (*render_pass_unbind)(const render_pass& _render_pass, const command_buffer& _command_buffer) = nullptr;
        void (*render_pass_destroy)(render_pass& _render_pass, const device& _device) = nullptr;
    };
} // namespace mars