#pragma once

#include <mars/graphics/backend/instance.hpp>

namespace mars {
    struct graphics_backend_functions;

    struct device {
        graphics_backend_functions* engine;
        void* data = nullptr;
    };

    struct device_impl {
        device (*device_create)(instance& _instance, window& _window) = nullptr;
        void (*device_destroy)(device& _device) = nullptr;
    };
} // namespace mars