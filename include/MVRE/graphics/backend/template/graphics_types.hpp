#ifndef __MVRE__GRAPHICS__TYPES__
#define __MVRE__GRAPHICS__TYPES__

#include <string>
#include <MVRE/math/vector4.hpp>
#include <MVRE/math/vector2.hpp>

namespace mvre_graphics {

    enum MVRE_MEMORY_TYPE {
        MVRE_MEMORY_TYPE_VERTEX,
        MVRE_MEMORY_TYPE_INDEX,
        MVRE_MEMORY_TYPE_TRANSFER,
        MVRE_MEMORY_TYPE_UNIFORM,
    };

    enum MVRE_RESOURCE_TYPE {
        MVRE_RESOURCE_TYPE_SHADER,
        MVRE_RESOURCE_TYPE_TEXTURE,
        MVRE_RESOURCE_TYPE_MATERIAL
    };

    enum MVRE_MATERIAL_INPUT {
        MVRE_MATERIAL_INPUT_SHADER,
        MVRE_MATERIAL_INPUT_TEXTURE
    };

    enum MVRE_SHADER_TYPE {
        MVRE_SHADER_TYPE_UNDEFINED,
        MVRE_SHADER_TYPE_VERTEX,
        MVRE_SHADER_TYPE_FRAGMENT
    };

    enum MVRE_SHADER_TOKEN {
        MVRE_SHADER_TOKEN_VERTEX,
        MVRE_SHADER_TOKEN_FRAGMENT,
        MVRE_SHADER_TOKEN_ATTRIBUTE,
        MVRE_SHADER_TOKEN_UNIFORM,
    };

    enum MVRE_UNIFORM_TYPE {
        MVRE_UNIFORM_TYPE_FLOAT,
        MVRE_UNIFORM_TYPE_MATRIX,
        MVRE_UNIFORM_TYPE_SAMPLER
    };

    enum MVRE_SHADER_INPUT_TYPE {
        MVRE_SHADER_INPUT_TYPE_SF_RG,
        MVRE_SHADER_INPUT_TYPE_SF_RGB,
        MVRE_SHADER_INPUT_TYPE_SF_RGBA,
    };

    struct mvre_shader_uniform {
        std::string name;
        MVRE_UNIFORM_TYPE type;
        size_t size;

        mvre_shader_uniform() = default;

        mvre_shader_uniform(std::string _name, MVRE_UNIFORM_TYPE _type, size_t _size) {
            name = _name;
            type = _type;
            size = _size;
        }
    };

    struct mvre_shader_input {
    public:
        int stride;
        unsigned int offset;
        MVRE_SHADER_INPUT_TYPE type;
    };

    struct mvre_shader_inputs {
    public:
        mvre_shader_input* input_data;
        size_t length;
    };

    struct mvre_viewport {
    public:
        mvre_math::vector2<int> position;
        mvre_math::vector2<size_t> size;
        mvre_math::vector2<float> depth;
    };

    class graphics_types {
    public:
        static MVRE_SHADER_TYPE token_to_type(MVRE_SHADER_TOKEN _token);
    };
}

#endif