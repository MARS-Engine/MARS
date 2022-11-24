#ifndef __MVRE__VERTEX2__
#define __MVRE__VERTEX2__

#include "MVRE/math/vector2.hpp"
#include "MVRE/math/vector3.hpp"
#include <MVRE/graphics/backend/template/shader_input.hpp>

namespace mvre_graphics {

    class vertex2 {
    private:
        static mvre_shader_inputs m_description;
    public:
        mvre_math::vector3<float> position;
        mvre_math::vector2<float> uv;

        static mvre_shader_inputs get_description() { return m_description; };
    };
}

#endif