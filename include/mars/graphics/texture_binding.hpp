#pragma once

#include "texture.hpp"
#include "texture_buffer.hpp"

namespace mars::graphics {
    template <typename T>
    void bind_textures(T& _material) {
        static constexpr auto ctx = std::meta::access_context::current();

        template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
            if constexpr (mars::meta::get_annotation<texture_prop_annotation>(mem).has_value()) {
                glActiveTexture(GL_TEXTURE0 + _material.[:mem:].location);
                glBindTexture(GL_TEXTURE_2D, _material.[:mem:].id);
            } else if constexpr (mars::meta::get_annotation<texture_buffer_prop_annotation>(mem).has_value()) {
                glBindTextureUnit(_material.[:mem:].location, _material.[:mem:].texture_id);
            }
        }
    }

    void bind_texture(const texture& _texture, size_t _location = 0) {
        glActiveTexture(GL_TEXTURE0 + _location);
        glBindTexture(GL_TEXTURE_2D, _texture.id);
    }
} // namespace mars::graphics