#ifndef MARS_VERTEX3_
#define MARS_VERTEX3_

#include "MARS/math/vector2.hpp"
#include "MARS/math/vector3.hpp"
#include <MARS/graphics/backend/template/shader_input.hpp>

namespace mars_graphics {

    class vertex3 {
    private:
        static mars_shader_inputs m_description;
    public:
        mars_math::vector3<float> position;
        mars_math::vector3<float> normal;
        mars_math::vector2<float> uv;

        static mars_shader_inputs& get_description() { return m_description; };
    };
}

#endif