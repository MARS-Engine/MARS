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

        static mars_shader_inputs description() {
            return {
                .input_data = std::vector<mars_shader_input>({
                    { 3, offsetof(vertex1, position), MARS_SHADER_INPUT_TYPE_SF_RGB },
                    }),
                .length = 1
            };
        };
    };
}

#endif