#pragma once

#include <mars/meta/type_erasure.hpp>

#include <cstddef>

enum mars_buffer_type {
    MARS_BUFFER_TYPE_VERTEX,
    MARS_BUFFER_TYPE_INDEX,
};

namespace mars {
    struct device;
    struct command_buffer;
    struct graphics_backend_functions;

    struct buffer {
        graphics_backend_functions* engine;
        meta::type_erasure_ptr data;
    };

    struct buffer_create_params {
        mars_buffer_type type;
        size_t alocated_size;
    };

    struct buffer_impl {
        buffer (*buffer_create)(const device& _device, const buffer_create_params& _params) = nullptr;
        void (*buffer_bind)(buffer& _buffer, const command_buffer& _command_buffer) = nullptr;
        void (*buffer_bind_index)(buffer& _buffer, const command_buffer& _command_buffer) = nullptr;
        void* (*buffer_map)(buffer& _buffer, const device& _device, size_t _size, size_t _offset) = nullptr;
        void (*buffer_unmap)(buffer& _buffer, const device& _device) = nullptr;
        void (*buffer_destroy)(buffer& _buffer, const device& _device) = nullptr;
    };
} // namespace mars