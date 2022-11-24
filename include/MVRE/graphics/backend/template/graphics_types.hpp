#ifndef __MVRE__GRAPHICS__TYPES__
#define __MVRE__GRAPHICS__TYPES__

#include <string>

namespace mvre_graphics {

    enum MVRE_MEMORY_TYPE {
        MVRE_MEMORY_TYPE_VERTEX,
        MVRE_MEMORY_TYPE_INDEX
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

    struct mvre_shader_uniform {
        std::string name;
        MVRE_UNIFORM_TYPE type;
        size_t size;

        mvre_shader_uniform(std::string _name, MVRE_UNIFORM_TYPE _type, size_t _size) {
            name = _name;
            type = _type;
            size = _size;
        }

        virtual void update(void* _data) { }
    };

    struct mvre_shader_input {
    public:
        int stride;
    };

    struct mvre_shader_inputs {
    public:
        mvre_shader_input* input_data;
        size_t length;
    };

    class graphics_types {
    public:
        static MVRE_SHADER_TYPE token_to_type(MVRE_SHADER_TOKEN _token);
    };
}

#endif