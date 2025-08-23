#pragma once

#include <cstddef>

enum mars_buffer_type {
    MARS_BUFFER_TYPE_VERTEX,
    MARS_BUFFER_TYPE_INDICE
};

enum mars_buffer_map {
    MARS_BUFFER_MAP_STATIC,
    MARS_BUFFER_MAP_DYNAMIC,
};

namespace mars {
    struct graphics_engine;

    struct buffer {
        graphics_engine* engine;
        void* data = nullptr;
        size_t size;
    };

    struct buffer_desc {
        mars_buffer_type type;
        size_t binding_divisor;
        size_t location;
    };

    struct buffer_impl {
        void* (*allocator)() = nullptr;
        void (*create_buffer)(buffer& _buffer, buffer_desc* _descriptions, size_t _desc_size) = nullptr;
    };
} // namespace mars