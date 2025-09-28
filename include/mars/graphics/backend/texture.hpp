#pragma once

#include <cstddef>
#include <mars/math/vector2.hpp>
#include <mars/meta/type_erasure.hpp>

enum mars_texture_format_type {
    MARS_TEXTURE_FORMAT_RGBA8_UNORM,
    MARS_TEXTURE_FORMAT_RGBA8_SRGB
};

namespace mars {
    struct graphics_backend_functions;
    struct device;
    struct buffer;
    struct command_buffer;

    struct texture {
        graphics_backend_functions* engine;
        meta::type_erasure_ptr data;
        vector2<size_t> size;
        unsigned char channels;
    };

    struct texture_create_params {
        vector2<size_t> size;
        mars_texture_format_type format;
    };

    struct texture_impl {
        texture (*texture_create)(const device& _device, const texture_create_params& _params) = nullptr;
        void (*texture_copy)(texture& _texture, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset);
        void (*texture_destroy)(texture& _texture, const device& _device) = nullptr;
    };
} // namespace mars