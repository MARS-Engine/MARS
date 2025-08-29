#pragma once

namespace mars {
    struct device;
    struct swapchain;
    class graphics_backend_functions;

    struct render_pass {
        graphics_backend_functions* engine;
        void* data = nullptr;
    };

    struct render_pass_impl {
        render_pass (*render_pass_create)(const device& _device, const swapchain& _swapchain) = nullptr;
        void (*render_pass_destroy)(render_pass& _render_pass, const device& _device) = nullptr;
    };
} // namespace mars