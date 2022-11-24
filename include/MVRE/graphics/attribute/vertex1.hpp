#ifndef __MVRE__VERTEX1__
#define __MVRE__VERTEX1__

#include <MVRE/graphics/backend/template/shader_input.hpp>
#include <MVRE/math/vector3.hpp>

namespace mvre_graphics {

    struct vertex1 {
    private:
        static mvre_shader_inputs m_description;
    public:
        mvre_math::vector3<float> position;

        explicit vertex1(mvre_math::vector3<float> _position) { position = _position; }

        static mvre_shader_inputs get_description() { return m_description; };
    };
}

#endif