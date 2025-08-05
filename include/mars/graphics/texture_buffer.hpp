#pragma once

#include <glad/gl.h>
#include <mars/math/vector2.hpp>
#include <mars/meta.hpp>
#include <meta>

#include "texture.hpp"

namespace mars {
    namespace graphics {

        struct texture_buffer_prop_annotation {
            size_t location;
        };

        struct texture_buffer {
            unsigned int texture_id;
            unsigned int buffer_id;
            size_t size;
            size_t location;
        };

        template <typename T>
        texture_buffer make_texture_buffer(size_t _size, mars_texture_format _format) {
            texture_buffer result;
            result.size = sizeof(T) * _size;
            result.location = 1;
            glCreateTextures(GL_TEXTURE_BUFFER, 1, &result.texture_id);
            glCreateBuffers(1, &result.buffer_id);

            glNamedBufferStorage(result.buffer_id, result.size, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
            glTextureBuffer(result.texture_id, __detail::texture_format_to_gl_internal(_format), result.buffer_id);

            return result;
        };

        template <typename T>
        T* map_texture_buffer(const texture_buffer& _buffer) {
            return static_cast<T*>(glMapNamedBufferRange(_buffer.buffer_id, 0, _buffer.size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
        }

        void unmap_texture_buffer(const texture_buffer& _buffer) {
            glUnmapNamedBuffer(_buffer.buffer_id);
        }

        template <typename T>
        void bind_texture_buffer(T& _material) {
            static constexpr auto ctx = std::meta::access_context::current();

            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
                glBindTextureUnit(_material.[:mem:].location, _material.[:mem:].texture_id);
            }
        }
    } // namespace graphics

    namespace prop {
        static consteval mars::graphics::texture_buffer_prop_annotation texture_buffer(size_t _location = 0) {
            return { _location };
        }
    } // namespace prop
} // namespace mars