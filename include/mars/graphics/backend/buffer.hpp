#pragma once

#include <mars/meta/type_erased.hpp>

#include <cstddef>

enum mars_buffer_type {
    MARS_BUFFER_TYPE_VERTEX = 1 << 1,
    MARS_BUFFER_TYPE_INDEX = 1 << 2,
    MARS_BUFFER_TYPE_UNIFORM = 1 << 3,
    MARS_BUFFER_TYPE_TRANSFER_SRC = 1 << 4,
    MARS_BUFFER_TYPE_TRANSFER_DST = 1 << 5,
};

enum mars_buffer_property {
    MARS_BUFFER_PROPERTY_HOST_VISIBLE = 1 << 1,
    MARS_BUFFER_PROPERTY_DEVICE_LOCAL = 1 << 2,
};

namespace mars {
    struct device;
    struct command_buffer;
    struct graphics_backend_functions;

    struct buffer {
        graphics_backend_functions* engine;
        meta::type_erased_ptr data;
        size_t allocated_size;
    };

    struct buffer_create_params {
        uint32_t buffer_type;
        uint32_t buffer_property;
        size_t allocated_size;
    };

    struct buffer_impl {
        buffer (*buffer_create)(const device& _device, const buffer_create_params& _params) = nullptr;
        void (*buffer_bind)(buffer& _buffer, const command_buffer& _command_buffer) = nullptr;
        void (*buffer_bind_index)(buffer& _buffer, const command_buffer& _command_buffer) = nullptr;
        void (*buffer_copy)(buffer& _buffer, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset) = nullptr;
        void* (*buffer_map)(buffer& _buffer, const device& _device, size_t _size, size_t _offset) = nullptr;
        void (*buffer_unmap)(buffer& _buffer, const device& _device) = nullptr;
        void (*buffer_destroy)(buffer& _buffer, const device& _device) = nullptr;
    };
} // namespace mars