#include "MVRE/graphics/attribute/vertex1.hpp"

using namespace mvre_graphics;
using namespace mvre_graphics_base;

mvre_graphics_base::mvre_shader_inputs vertex1::m_description = {
    .input_data = new mvre_shader_input[1]{
        { 3 },
    },
    .length = 1
};