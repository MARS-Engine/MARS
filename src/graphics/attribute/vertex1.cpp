#include <MVRE/graphics/attribute/vertex1.hpp>

using namespace mvre_graphics;

mvre_shader_inputs vertex1::m_description = {
    .input_data = new mvre_shader_input[1]{
        { 3, offsetof(vertex1, position), MVRE_SHADER_INPUT_TYPE_SF_RGB },
    },
    .length = 1
};