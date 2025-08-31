#pragma once

#include "shader.hpp"

#include <mars/math/vector2.hpp>
#include <mars/meta/type_erasure.hpp>

namespace mars {
    struct render_pass;
    struct command_buffer;

    struct pipeline {
        graphics_backend_functions* engine;
        meta::type_erasure_ptr data;
    };

    struct viewport {
        mars::vector2<size_t> size;
    };

    // todo: add support for custom stage info pname
    struct pipeline_setup {
        shader pipeline_shader;
        viewport view;
    };

    struct pipeline_bind_params {
        mars::vector2<size_t> size;
    };

    struct pipeline_impl {
        pipeline (*pipeline_create)(const device& _device, const render_pass& _render_pass, const pipeline_setup& _setup) = nullptr;
        void (*pipeline_bind)(const pipeline& _pipeline, const command_buffer& _command_buffer, const pipeline_bind_params& _params) = nullptr;
        void (*pipeline_destroy)(pipeline& _pipeline, const device& _device) = nullptr;
    };
} // namespace mars