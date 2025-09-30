#pragma once

#include <mars/meta/type_erased.hpp>

#include <cstddef>
#include <vector>

namespace mars {
    struct device;
    struct graphics_backend_functions;

    struct command_pool {
        graphics_backend_functions* engine = nullptr;
        meta::type_erased_ptr data;
    };

    struct command_buffer {
        graphics_backend_functions* engine = nullptr;
        meta::type_erased_ptr data;
        size_t buffer_index = 0;
    };

    struct command_buffer_draw_params {
        size_t vertex_count;
        size_t instance_count;
        size_t first_vertex;
        size_t first_instance;
    };

    struct command_buffer_draw_indexed_params {
        size_t index_count;
        size_t instance_count;
        size_t first_index;
        size_t vertex_offset;
        size_t first_instance;
    };

    struct command_pool_impl {
        command_pool (*command_pool_create)(const device& _device) = nullptr;
        std::vector<command_buffer> (*command_buffer_create)(const command_pool& _command_pool, const device& _device, size_t _n_command_buffers) = nullptr;
        void (*command_buffer_reset)(const command_buffer& _command_buffer) = nullptr;
        void (*command_buffer_record)(const command_buffer& _command_buffer) = nullptr;
        void (*command_buffer_record_end)(const command_buffer& _command_buffer) = nullptr;
        void (*command_buffer_draw)(const command_buffer& _command_buffer, const command_buffer_draw_params& _params) = nullptr;
        void (*command_buffer_draw_indexed)(const command_buffer& _command_buffer, const command_buffer_draw_indexed_params& _params) = nullptr;
        void (*command_pool_destroy)(command_pool& _command_pool, const device& _device) = nullptr;
    };
} // namespace mars