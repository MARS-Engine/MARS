#ifndef __MVRE__VERTEX3__
#define __MVRE__VERTEX3__

#include "MVRE/math/vector2.hpp"
#include "MVRE/math/vector3.hpp"
#include "MVRE/graphics/backend/base/base_types.hpp"

namespace mvre_graphics {

    class vertex3 {
    private:
        static mvre_graphics_base::mvre_shader_inputs m_description;
    public:
        mvre_math::vector3<float> position;
        mvre_math::vector3<float> normal;
        mvre_math::vector2<float> uv;

        static mvre_graphics_base::mvre_shader_inputs get_description() { return m_description; };
    };
}

#endif