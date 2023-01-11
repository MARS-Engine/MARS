#ifndef MARS_VERTEX1_
#define MARS_VERTEX1_

#include <MARS/graphics/backend/template/shader_input.hpp>
#include <MARS/math/vector3.hpp>

namespace mars_graphics {

    struct vertex1 {
    private:
        static mars_shader_inputs m_description;
    public:
        mars_math::vector3<float> position;

        explicit vertex1(const mars_math::vector3<float>& _position) { position = _position; }

        static mars_shader_inputs& get_description() { return m_description; };
    };
}

#endif