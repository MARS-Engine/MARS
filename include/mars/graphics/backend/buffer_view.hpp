#pragma once

#include <cstddef>
#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/format.hpp>
#include <mars/math/vector2.hpp>
#include <mars/meta/type_erased.hpp>
#include <mars/meta/type_erased_fn.hpp>

namespace mars {
    struct graphics_backend_functions;
    struct device;
    struct command_buffer;

    struct buffer_view {
        graphics_backend_functions* engine;
        meta::type_erased_ptr data;
        buffer src_buffer;
    };

    struct buffer_view_create_params {
        buffer src_buffer;
        mars_format_type format;
    };

    struct buffer_view_impl {
        buffer_view (*buffer_view_create)(const device& _device, const buffer_view_create_params& _params) = nullptr;
        void (*buffer_view_destroy)(buffer_view& _buffer_view, const device& _device) = nullptr;
    };
} // namespace mars