#ifndef MARS_VERTEX2_
#define MARS_VERTEX2_

#include "MARS/math/vector2.hpp"
#include "MARS/math/vector3.hpp"
#include <MARS/graphics/backend/template/shader_input.hpp>

namespace mars_graphics {

    class vertex2 {
    private:
        static mars_shader_inputs m_description;
    public:
        mars_math::vector3<float> position;
        mars_math::vector2<float> uv;

        static std::shared_ptr<mars_shader_inputs> get_description() { return std::make_shared<mars_shader_inputs>(m_description); };
    };
}

#endif