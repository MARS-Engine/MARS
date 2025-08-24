#pragma once

#include "instance.hpp"

namespace mars {
    struct graphics_backend_functions;
    struct window;

    struct swapchain {
        graphics_backend_functions* engine;
        void* data = nullptr;
    };

    struct swapchain_impl {
        swapchain (*swapchain_create)(instance& _instance, window& _window) = nullptr;
        void (*swapchain_destroy)(swapchain& _swapchain, instance& _instance) = nullptr;
    };
} // namespace mars