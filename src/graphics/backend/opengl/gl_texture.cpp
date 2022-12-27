#include <MVRE/graphics/backend/opengl/gl_texture.hpp>

using namespace mvre_graphics;

bool gl_texture::load_resource(const std::string& _texture_path) {
    if (!load_texture(_texture_path)) {
        free_og_texture();
        return false;
    }

    glGenTextures(1, &m_texture_id);
    bind();

    //TODO: This
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_size.x(), m_size.y(), 0, GL_RGB + m_channels - 3, GL_UNSIGNED_BYTE, m_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    free_og_texture();
    return true;
}

void gl_texture::clean() {
    glDeleteTextures(1, &m_texture_id);
    m_texture_id = 0;
}