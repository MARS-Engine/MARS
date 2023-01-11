#include <MARS/graphics/attribute/vertex3.hpp>

using namespace mars_graphics;

mars_shader_inputs vertex3::m_description = {
    .input_data = new mars_shader_input[3]{
        { 3, offsetof(vertex3, position), MARS_SHADER_INPUT_TYPE_SF_RGB },
        { 3, offsetof(vertex3, normal), MARS_SHADER_INPUT_TYPE_SF_RGB },
        { 2, offsetof(vertex3, uv), MARS_SHADER_INPUT_TYPE_SF_RG }
    },
    .length = 3
};