#pragma once

#include <SDL3/SDL_opengl.h>
#include <cstddef>
#include <mars/math/annotation.hpp>
#include <mars/meta.hpp>
#include <meta>

namespace mars::graphics {

    consteval GLenum c_type_to_gl_type(std::meta::info _type) {
        if (mars::meta::is_type<uint8_t>(_type))
            return GL_UNSIGNED_BYTE;
        if (mars::meta::is_type<int8_t>(_type))
            return GL_BYTE;
        if (mars::meta::is_type<uint16_t>(_type))
            return GL_UNSIGNED_SHORT;
        if (mars::meta::is_type<int16_t>(_type))
            return GL_SHORT;
        if (mars::meta::is_type<uint32_t>(_type))
            return GL_UNSIGNED_INT;
        if (mars::meta::is_type<int32_t>(_type))
            return GL_INT;
        if (mars::meta::is_type<float>(_type))
            return GL_FLOAT;
        if (mars::meta::is_type<double>(_type))
            return GL_INT;
        return GL_NONE;
    }

    // check if its a valid vector or matrix
    consteval bool is_valid_gl_struct_type(std::meta::info _type) {
        GLenum attribute_type = GL_NONE;
        auto ctx = std::meta::access_context::current();

        if (mars::meta::get_annotation<mars::matrix_annotation>(std::meta::type_of(_type)).has_value())
            return true;
        else {
            for (auto sub_mem : std::meta::nonstatic_data_members_of(std::meta::type_of(_type), ctx)) {
                GLenum type = c_type_to_gl_type(sub_mem);

                if (type == GL_NONE)
                    return false;
                else
                    attribute_type = type;

                if (attribute_type != type)
                    return false;
            }
        }
        return true;
    }

} // namespace mars::graphics