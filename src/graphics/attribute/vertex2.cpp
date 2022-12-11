#include <MVRE/graphics/attribute/vertex2.hpp>

using namespace mvre_graphics;

mvre_shader_inputs vertex2::m_description = {
    .input_data = new mvre_shader_input[2]{
        { 3, offsetof(vertex2, position), MVRE_SHADER_INPUT_TYPE_SF_RGB },
        { 2, offsetof(vertex2, uv), MVRE_SHADER_INPUT_TYPE_SF_RG }
    },
    .length = 2
};