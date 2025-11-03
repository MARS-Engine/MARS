#pragma once

#include <cstddef>
#include <mars/graphics/backend/format.hpp>
#include <mars/math/vector2.hpp>
#include <mars/meta/type_erased.hpp>
#include <mars/meta/type_erased_fn.hpp>

namespace mars {
    struct graphics_backend_functions;
    struct device;
    struct buffer;
    struct command_buffer;

    struct texture_view {
        meta::type_erased_ptr data;
        meta::type_erased_fn view_extractor;
    };

    struct texture {
        graphics_backend_functions* engine;
        meta::type_erased_ptr data;
        texture_view view;
        vector2<size_t> size;
        unsigned char channels;
        unsigned char format_size;
    };

    struct texture_create_params {
        vector2<size_t> size;
        mars_format_type format;
        mars_texture_usage usage = MARS_TEXTURE_USAGE_SAMPLED | MARS_TEXTURE_USAGE_TRANSFER_DST;
        mars_texture_filter_mode filter = MARS_TEXTURE_FILTER_LINEAR;
        // number of textures in this single textures, good for sampler arrays, DO NOT USE THIS FOR 3D TEXTURES
        size_t array_size = 1;
    };

    struct texture_impl {
        texture (*texture_create)(const device& _device, const texture_create_params& _params) = nullptr;
        void (*texture_copy)(texture& _texture, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset);
        void (*texture_destroy)(texture& _texture, const device& _device) = nullptr;
    };
} // namespace mars