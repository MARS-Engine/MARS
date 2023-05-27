#ifndef MARS_VERTEX3_
#define MARS_VERTEX3_

#include "MARS/math/vector2.hpp"
#include "MARS/math/vector3.hpp"
#include <MARS/graphics/backend/template/shader_input.hpp>

namespace mars_graphics {

    class vertex3 {
    public:
        mars_math::vector3<float> position;
        mars_math::vector3<float> normal;
        mars_math::vector2<float> uv;

        static mars_shader_inputs description() {
            return {
                .input_data = std::vector<mars_shader_input>({
                    { 3, offsetof(vertex3, position), MARS_SHADER_INPUT_TYPE_SF_RGB },
                    { 3, offsetof(vertex3, normal), MARS_SHADER_INPUT_TYPE_SF_RGB },
                    { 2, offsetof(vertex3, uv), MARS_SHADER_INPUT_TYPE_SF_RG }
                }),
                .length = 3
            };
        };
    };
}

#endif