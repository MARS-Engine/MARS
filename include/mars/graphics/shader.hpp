#pragma once

#include <glad/gl.h>
#include <mars/io/file.hpp>
#include <mars/meta.hpp>
#include <meta>
#include <vector>

enum mars_shader_type {
    MARS_SHADER_TYPE_VERTEX,
    MARS_SHADER_TYPE_FRAGMENT
};

namespace mars {
    namespace graphics {
        constexpr GLenum buffer_type_to_gl(mars_shader_type _type) {
            switch (_type) {
            case MARS_SHADER_TYPE_VERTEX:
                return GL_VERTEX_SHADER;
            case MARS_SHADER_TYPE_FRAGMENT:
                return GL_FRAGMENT_SHADER;
            }
        }

        struct shader_prop_annotation {
            mars_shader_type type;
            const char* name;
        };

        struct shader {
            unsigned int id;
        };

        struct program {
            unsigned int id;
        };

        template <typename T>
        program make_program(T& _pipeline) {
            static constexpr auto ctx = std::meta::access_context::current();

            program result;

            result.id = glCreateProgram();

            int success;
            char infoLog[512];

            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                constexpr std::optional<shader_prop_annotation> annoation = mars::meta::get_annotation<shader_prop_annotation>(mem);
                if constexpr (annoation.has_value()) {
                    std::string str = io::read_file((std::string("data/shaders/")) + annoation.value().name);
                    const char* shader = str.c_str();
                    unsigned int shader_id = glCreateShader(buffer_type_to_gl(annoation.value().type));
                    glShaderSource(shader_id, 1, &shader, NULL);
                    glCompileShader(shader_id);

                    // check for shader compile errors
                    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
                    if (!success) {
                        glGetShaderInfoLog(shader_id, 512, NULL, infoLog);
                        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                                  << infoLog << std::endl;
                    }

                    glAttachShader(result.id, shader_id);
                    _pipeline.[:mem:].id = shader_id;
                }
            }

            glLinkProgram(result.id);

            glGetProgramiv(result.id, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(result.id, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                          << infoLog << std::endl;
            }

            return result;
        }

        void use_program(const program& _program) {
            glUseProgram(_program.id);
        }

        void destroy_program(program& _program) {
            glDeleteProgram(_program.id);
            _program = {};
        }
    } // namespace graphics

    namespace prop {
        static consteval mars::graphics::shader_prop_annotation shader(const char* _name, mars_shader_type _type) { return { _type, std::define_static_string(_name) }; }
        static consteval mars::graphics::shader_prop_annotation shader_frag(const char* _name) { return shader(_name, MARS_SHADER_TYPE_FRAGMENT); }
        static consteval mars::graphics::shader_prop_annotation shader_vert(const char* _name) { return shader(_name, MARS_SHADER_TYPE_VERTEX); }
    } // namespace prop

} // namespace mars