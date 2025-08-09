#pragma once

#include <imgui.h>
#include <stb/stb_image.h>
#include <vector>

#include <glad/gl.h>
#include <iostream>
#include <mars/event/event.hpp>
#include <mars/imgui/struct_editor.hpp>
#include <mars/io/file.hpp>
#include <mars/math/vector2.hpp>
#include <mars/meta.hpp>
#include <mars/parser/json/json.hpp>
#include <meta>

enum mars_texture_format {
    MARS_TEXTURE_FORMAT_RED,
    MARS_TEXTURE_FORMAT_RG,
    MARS_TEXTURE_FORMAT_RGB,
    MARS_TEXTURE_FORMAT_RGBA,
    MARS_TEXTURE_FORMAT_RGBF,
    MARS_TEXTURE_FORMAT_RF,
    MARS_TEXTURE_FORMAT_R_UI
};

namespace mars {
    namespace graphics {

        namespace __detail {
            inline constexpr GLenum texture_format_to_gl_size(mars_texture_format _format) {
                switch (_format) {
                case MARS_TEXTURE_FORMAT_RED:
                case MARS_TEXTURE_FORMAT_RG:
                case MARS_TEXTURE_FORMAT_RGB:
                case MARS_TEXTURE_FORMAT_RGBA:
                    return GL_UNSIGNED_BYTE;
                case MARS_TEXTURE_FORMAT_RF:
                case MARS_TEXTURE_FORMAT_RGBF:
                    return GL_FLOAT;
                case MARS_TEXTURE_FORMAT_R_UI:
                    return GL_UNSIGNED_INT;
                }
                return GL_NONE;
            }

            inline constexpr GLenum texture_format_to_gl(mars_texture_format _format) {
                switch (_format) {
                case MARS_TEXTURE_FORMAT_RED:
                case MARS_TEXTURE_FORMAT_RF:
                    return GL_RED;
                case MARS_TEXTURE_FORMAT_RG:
                    return GL_RG;
                case MARS_TEXTURE_FORMAT_RGB:
                case MARS_TEXTURE_FORMAT_RGBF:
                    return GL_RGB;
                case MARS_TEXTURE_FORMAT_RGBA:
                    return GL_RGBA;
                case MARS_TEXTURE_FORMAT_R_UI:
                    return GL_RED_INTEGER;
                }
                return GL_NONE;
            }

            inline constexpr GLenum texture_format_to_gl_internal(mars_texture_format _format) {
                switch (_format) {
                case MARS_TEXTURE_FORMAT_RED:
                    return GL_R8;
                case MARS_TEXTURE_FORMAT_RG:
                    return GL_RG8;
                case MARS_TEXTURE_FORMAT_RGB:
                    return GL_RGB8;
                case MARS_TEXTURE_FORMAT_RGBA:
                    return GL_RGBA8;
                case MARS_TEXTURE_FORMAT_R_UI:
                    return GL_R32UI;
                case MARS_TEXTURE_FORMAT_RGBF:
                    return GL_RGB32F;
                case MARS_TEXTURE_FORMAT_RF:
                    return GL_R32F;
                }
                return GL_NONE;
            }
        } // namespace __detail

        struct texture_prop_annotation {
            const char* name;
            size_t location;
            mars_texture_format format;
        };

        struct texture {
            std::string path;
            size_t location;
            mars_texture_format format;

            [[= mars::json::skip]] unsigned int id;
            [[= mars::json::skip]] mars::vector2<size_t> size;
        };

        inline texture texture_load(void* _data, size_t _width, size_t _height, mars_texture_format _format, int _location) {
            texture result;
            result.location = _location;
            result.size = { _width, _height };

            glCreateTextures(GL_TEXTURE_2D, 1, &result.id);

            glTextureStorage2D(result.id, 1, __detail::texture_format_to_gl_internal(_format), _width, _height);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTextureSubImage2D(result.id, 0, 0, 0, _width, _height, __detail::texture_format_to_gl(_format), __detail::texture_format_to_gl_size(_format), _data);
            glTextureParameteri(result.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(result.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(result.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(result.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            return result;
        };

        inline texture texture_load(const std::string& _path, int _location) {
            int width, height, channels;
            unsigned char* data = stbi_load(_path.c_str(), &width, &height, &channels, 0);

            texture result;
            result.location = _location;
            result.size = { static_cast<size_t>(width), static_cast<size_t>(height) };
            result.path = _path;

            result.format = (mars_texture_format)(MARS_TEXTURE_FORMAT_RED + (channels - 1));

            glCreateTextures(GL_TEXTURE_2D, 1, &result.id);

            glTextureStorage2D(result.id, 1, __detail::texture_format_to_gl_internal(result.format), width, height);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTextureSubImage2D(result.id, 0, 0, 0, width, height, __detail::texture_format_to_gl(result.format), __detail::texture_format_to_gl_size(result.format), data);
            glTextureParameteri(result.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(result.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(result.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(result.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            stbi_image_free(data);

            return result;
        };

        template <typename T>
        void texture_load(T& _material) {
            static constexpr auto ctx = std::meta::access_context::current();

            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                constexpr std::optional<texture_prop_annotation> annoation = mars::meta::get_annotation<texture_prop_annotation>(mem);
                if constexpr (annoation.has_value()) {
                    std::string str = "data/textures/" + std::string(annoation.value().name);
                    _material.[:mem:].path = str;
                    _material.[:mem:].format = annoation.value().format;

                    int width, height, nrChannels;
                    unsigned char* data = stbi_load(str.c_str(), &width, &height, &nrChannels, 0);

                    _material.[:mem:].size = { (size_t)width, (size_t)height };

                    glCreateTextures(GL_TEXTURE_2D, 1, &_material.[:mem:].id);

                    glTextureStorage2D(_material.[:mem:].id, 1, __detail::texture_format_to_gl_internal(annoation.value().format), width, height);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                    if (data) {
                        glTextureSubImage2D(_material.[:mem:].id, 0, 0, 0, width, height, __detail::texture_format_to_gl(annoation.value().format), GL_UNSIGNED_BYTE, data);
                        glTextureParameteri(_material.[:mem:].id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTextureParameteri(_material.[:mem:].id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        glTextureParameteri(_material.[:mem:].id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTextureParameteri(_material.[:mem:].id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    } else
                        std::cout << "Failed to load texture" << std::endl;

                    stbi_image_free(data);
                }
            }
        }

        inline texture texture_duplicate(const texture& _other) {
            texture result = _other;
            glCreateTextures(GL_TEXTURE_2D, 1, &result.id);
            glTextureStorage2D(result.id, 1, __detail::texture_format_to_gl_internal(result.format), result.size.x, result.size.y);
            glCopyImageSubData(
                _other.id, GL_TEXTURE_2D, 0, 0, 0, 0,
                result.id, GL_TEXTURE_2D, 0, 0, 0, 0,
                result.size.x, result.size.y, 1);
            return result;
        }

        inline void texture_destroy(texture& _texture) {
            glDeleteTextures(1, &_texture.id);
            _texture = {};
        }
    } // namespace graphics

    namespace json {
        template <>
        struct json_type_parser<mars::graphics::texture> : public json_type_parser_base<mars::graphics::texture> {
            inline static std::string_view::iterator parse(const std::string_view& _json, mars::graphics::texture& _value) {
                auto result = json_type_parser_base<mars::graphics::texture>::default_parse(_json, _value);
                _value = mars::graphics::texture_load(_value.path, _value.location);
                return result;
            }

            inline static void stringify(mars::graphics::texture& _value, std::string& _out) {
                json_type_parser_base<mars::graphics::texture>::default_stringify(_value, _out);
            }

            static constexpr bool struct_support = true;
        };
    }; // namespace json

    namespace prop {
        static consteval mars::graphics::texture_prop_annotation texture(const char* _name, size_t _location = 0, mars_texture_format _format = MARS_TEXTURE_FORMAT_RGB) {
            return { std::define_static_string(_name), _location, _format };
        }
    } // namespace prop
} // namespace mars