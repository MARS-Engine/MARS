#ifndef MARS_SHADER_INPUT_BUILDER_
#define MARS_SHADER_INPUT_BUILDER_

#include <MARS/graphics/backend/template/graphics_types.hpp>
#include "graphics_builder.hpp"
#include <map>

namespace mars_graphics {
    class buffer;
    class shader_input;

    class shader_input_builder : graphics_builder<shader_input> {
    public:
        shader_input_builder() = delete;

        using graphics_builder::graphics_builder;

        std::shared_ptr<buffer> add_buffer(size_t _input_size, MARS_MEMORY_TYPE _input_type);

        void load_input(const mars_shader_inputs& _inputs);

        std::shared_ptr<shader_input> build();
    };
}

#endif