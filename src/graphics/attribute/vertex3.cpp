#include <MVRE/graphics/attribute/vertex3.hpp>

using namespace mvre_graphics;

mvre_shader_inputs vertex3::m_description = {
    .input_data = new mvre_shader_input[3]{
        { 3, offsetof(vertex3, position), MVRE_SHADER_INPUT_TYPE_SF_RGB },
        { 3, offsetof(vertex3, normal), MVRE_SHADER_INPUT_TYPE_SF_RGB },
        { 2, offsetof(vertex3, uv), MVRE_SHADER_INPUT_TYPE_SF_RG }
    },
    .length = 3
};