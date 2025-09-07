#pragma once

#include "format.hpp"
#include "shader.hpp"

#include <mars/math/vector2.hpp>
#include <mars/meta/type_erasure.hpp>

enum mars_pipeline_input_advance_type {
    MARS_PIPELINE_INPUT_ADVANCE_TYPE_VERTEX,
    MARS_PIPELINE_INPUT_ADVANCE_TYPE_INSTANCE,
};

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

    struct pipeline_attribute_description {
        size_t binding;
        size_t location;
        size_t offset;
        graphics::format input_format;
    };

    struct pipeline_binding_description {
        size_t stride;
        size_t binding;
        mars_pipeline_input_advance_type type = MARS_PIPELINE_INPUT_ADVANCE_TYPE_VERTEX;
    };

    // todo: add support for custom stage info pname
    struct pipeline_setup {
        shader pipeline_shader;
        std::vector<pipeline_binding_description> bindings;
        std::vector<pipeline_attribute_description> attributes;
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