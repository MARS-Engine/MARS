#pragma once

#include "format.hpp"
#include "instance.hpp"
#include "texture.hpp"

namespace mars {
    struct graphics_backend_functions;
    struct window;
    class device;

    struct swapchain {
        graphics_backend_functions* engine;
        meta::type_erased_ptr data;
        size_t swapchain_size = 0;
        mars_format_type format;
        texture_view view;
    };

    struct swapchain_impl {
        swapchain (*swapchain_create)(const device& _device, const window& _window) = nullptr;
        void (*swapchain_destroy)(swapchain& _swapchain, const device& _device) = nullptr;
    };
} // namespace mars