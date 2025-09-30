#pragma once

#include <mars/math/vector2.hpp>
#include <mars/meta/type_erased.hpp>

#include <cstddef>

namespace mars {
    struct device;
    struct swapchain;
    struct render_pass;
    struct graphics_backend_functions;

    struct framebuffer {
        graphics_backend_functions* engine;
        meta::type_erased_ptr data;
        vector2<size_t> extent;
    };

    struct framebuffer_impl {
        framebuffer (*framebuffer_create)(const device& _device, const swapchain& _swapchain, const render_pass& _render_pass) = nullptr;
        void (*framebuffer_destroy)(framebuffer& _frame_buffer, const device& _device) = nullptr;
    };
} // namespace mars