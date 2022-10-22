#include "MVRE/graphics/attribute/vertex3.hpp"

using namespace mvre_graphics;
using namespace mvre_graphics_base;

mvre_graphics_base::mvre_shader_inputs vertex3::m_description = {
    .input_data = new mvre_shader_input[3]{
        { 3 },
        { 3 },
        { 2 }
    },
    .length = 3
};