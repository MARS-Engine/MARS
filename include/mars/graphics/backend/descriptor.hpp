#pragma once

#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/pipeline.hpp>
#include <vector>

namespace mars {
    struct descriptor {
        graphics_backend_functions* engine;
        meta::type_erasure_ptr data;
        size_t frames_in_flight;
    };

    struct descriptor_set {
        graphics_backend_functions* engine;
        meta::type_erasure_ptr data;
    };

    struct descriptor_set_create_params {
        std::vector<buffer> buffers;
    };

    struct descriptor_impl {
        descriptor (*descriptor_create)(const device& _device, size_t _frames_in_flight) = nullptr;
        descriptor_set (*descriptor_set_create)(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) = nullptr;
        void (*descriptor_set_bind)(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const pipeline& _pipeline, size_t _current_frame);
        void (*descriptor_destroy)(descriptor& _descriptor, const device& _device) = nullptr;
    };
} // namespace mars