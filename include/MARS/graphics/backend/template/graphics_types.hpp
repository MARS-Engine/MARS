#ifndef MARS_GRAPHICS_TYPES_
#define MARS_GRAPHICS_TYPES_

#include <string>
#include <MARS/math/vector4.hpp>
#include <MARS/math/vector2.hpp>

namespace mars_graphics {

    enum MARS_TOPOLOGY {
        MARS_TOPOLOGY_TRIANGLE_LIST,
        MARS_TOPOLOGY_TRIANGLE_STRIP
    };

    enum MARS_MEMORY_TYPE {
        MARS_MEMORY_TYPE_VERTEX,
        MARS_MEMORY_TYPE_INDEX,
        MARS_MEMORY_TYPE_TRANSFER,
        MARS_MEMORY_TYPE_UNIFORM,
    };

    enum MARS_RESOURCE_TYPE {
        MARS_RESOURCE_TYPE_ENGINE,
        MARS_RESOURCE_TYPE_SHADER,
        MARS_RESOURCE_TYPE_TEXTURE,
        MARS_RESOURCE_TYPE_MATERIAL,
        MARS_RESOURCE_TYPE_RENDERER
    };

    enum MARS_MATERIAL_INPUT {
        MARS_MATERIAL_INPUT_SHADER,
        MARS_MATERIAL_INPUT_TEXTURE
    };

    enum MARS_SHADER_TYPE {
        MARS_SHADER_TYPE_UNDEFINED,
        MARS_SHADER_TYPE_VERTEX,
        MARS_SHADER_TYPE_FRAGMENT
    };

    enum MARS_SHADER_TOKEN {
        MARS_SHADER_TOKEN_VERTEX,
        MARS_SHADER_TOKEN_FRAGMENT,
        MARS_SHADER_TOKEN_ATTRIBUTE,
        MARS_SHADER_TOKEN_UNIFORM,
    };

    enum MARS_UNIFORM_TYPE {
        MARS_UNIFORM_TYPE_FLOAT,
        MARS_UNIFORM_TYPE_MATRIX,
        MARS_UNIFORM_TYPE_BUFFER,
        MARS_UNIFORM_TYPE_SAMPLER
    };

    enum MARS_SHADER_INPUT_TYPE {
        MARS_SHADER_INPUT_TYPE_SF_RG,
        MARS_SHADER_INPUT_TYPE_SF_RGB,
        MARS_SHADER_INPUT_TYPE_SF_RGBA,
    };

    enum MARS_FORMAT {
        MARS_FORMAT_UNDEFINED,
        MARS_FORMAT_S_R8,
        MARS_FORMAT_S_RGB8,
        MARS_FORMAT_S_RGBA8,
        MARS_FORMAT_SBGRA_8,
        MARS_FORMAT_F_RGB16,
        MARS_FORMAT_F_RGBA16,
        MARS_FORMAT_DEPTH_32_SFLOAT,
        MARS_FORMAT_DEPTH_32_SFLOAT_STENCIL_8_UINT,
        MARS_FORMAT_DEPTH_24_UNORM_STENCIL_8_UINT
    };

    enum MARS_TEXTURE_USAGE {
        MARS_TEXTURE_USAGE_COLOR,
        MARS_TEXTURE_USAGE_DEPTH,
        MARS_TEXTURE_USAGE_TRANSFER
    };

    enum MARS_TEXTURE_LAYOUT {
        MARS_TEXTURE_LAYOUT_UNDEFINED,
        MARS_TEXTURE_LAYOUT_READONLY,
        MARS_TEXTURE_LAYOUT_COLOR_ATTACHMENT,
        MARS_TEXTURE_LAYOUT_PRESENT,
        MARS_TEXTURE_LAYOUT_DEPTH_OPTIMAL
    };

    enum MARS_TEXTURE_DATA_TYPE {
        MARS_TEXTURE_DATA_TYPE_FLOAT,
        MARS_TEXTURE_DATA_TYPE_UNSIGNED_BYTE
    };

    struct mars_shader_uniform {
        std::string name;
        MARS_UNIFORM_TYPE type;
        size_t size;
        size_t binding;

        mars_shader_uniform() = default;

        mars_shader_uniform(const std::string& _name, MARS_UNIFORM_TYPE _type, size_t _size, size_t _binding) {
            name = _name;
            type = _type;
            size = _size;
            binding = _binding;
        }
    };

    struct mars_shader_input {
    public:
        int stride;
        unsigned int offset;
        MARS_SHADER_INPUT_TYPE type;
    };

    struct mars_shader_inputs {
    public:
        mars_shader_input* input_data;
        size_t length;
    };

    struct mars_viewport {
    public:
        mars_math::vector2<int> position;
        mars_math::vector2<size_t> size;
        mars_math::vector2<float> depth;
    };

    class graphics_types {
    public:
        static MARS_SHADER_TYPE token_to_type(MARS_SHADER_TOKEN _token);
    };
}

#endif