#include <MARS/graphics/attribute/vertex2.hpp>

using namespace mars_graphics;

mars_shader_inputs vertex2::m_description = {
    .input_data = new mars_shader_input[2]{
        { 3, offsetof(vertex2, position), MARS_SHADER_INPUT_TYPE_SF_RGB },
        { 2, offsetof(vertex2, uv), MARS_SHADER_INPUT_TYPE_SF_RG }
    },
    .length = 2
};