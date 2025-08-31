#pragma once

#include <cstddef>
#include <mars/meta/type_erasure.hpp>

namespace mars {
    struct graphics_backend_functions;
    struct device;
    struct swapchain;

    struct sync {
        graphics_backend_functions* engine;
        meta::type_erasure_ptr data;
    };

    struct sync_impl {
        sync (*sync_create)(const device& _device, size_t _size) = nullptr;
        void (*sync_wait)(const sync& _sync, const device& _device, size_t _index) = nullptr;
        void (*sync_reset)(const sync& _sync, const device& _device, size_t _current_frame) = nullptr;
        bool (*sync_get_next_image)(const sync& _sync, const device& _device, const swapchain& _swapchain, size_t _current_frame, size_t& _image_index) = nullptr;
        void (*sync_destroy)(sync& _sync, const device& _device) = nullptr;
    };
} // namespace mars