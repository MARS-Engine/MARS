#include <MARS/graphics/attribute/vertex1.hpp>

using namespace mars_graphics;

mars_shader_inputs vertex1::m_description = {
    .input_data = new mars_shader_input[1]{
        { 3, offsetof(vertex1, position), MARS_SHADER_INPUT_TYPE_SF_RGB },
    },
    .length = 1
};