#include <MVRE/graphics/attribute/vertex2.hpp>

using namespace mvre_graphics;

mvre_shader_inputs vertex2::m_description = {
    .input_data = new mvre_shader_input[2]{
        { 3 },
        { 2 }
    },
    .length = 2
};