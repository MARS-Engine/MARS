#pragma once

#include <mars/graphics/backend/instance.hpp>

namespace mars {
    struct graphics_backend_functions;
    struct command_buffer;
    struct swapchain;
    struct sync;

    struct device {
        graphics_backend_functions* engine;
        void* data = nullptr;
    };

    struct device_impl {
        device (*device_create)(instance& _instance, window& _window) = nullptr;
        void (*device_submit_graphics_queue)(const device& _device, const sync& _sync, size_t _current_index, size_t _image_index, const command_buffer* _buffers, size_t _n_buffers) = nullptr;
        bool (*device_present)(const device& _device, const sync& _sync, const swapchain& _swapchain, size_t _image_index) = nullptr;
        void (*device_wait)(const device& _device) = nullptr;
        void (*device_destroy)(device& _device) = nullptr;
    };
} // namespace mars