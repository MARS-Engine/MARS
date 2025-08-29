#pragma once

#include "shader.hpp"

#include <mars/math/vector2.hpp>

namespace mars {
    class render_pass;

    struct pipeline {
        graphics_backend_functions* engine;
        void* data = nullptr;
    };

    struct viewport {
        mars::vector2<size_t> size;
    };

    // todo: add support for custom stage info pname
    struct pipeline_setup {
        shader pipeline_shader;
        viewport view;
    };

    struct pipeline_impl {
        pipeline (*pipeline_create)(const device& _device, const render_pass& _render_pass, const pipeline_setup& _setup) = nullptr;
        void (*pipeline_destroy)(pipeline& _pipeline, const device& _device) = nullptr;
    };
} // namespace mars