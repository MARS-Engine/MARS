#include <MARS/graphics/backend/opengl/gl_framebuffer.hpp>

using namespace mars_graphics;

void gl_framebuffer::add_texture(MARS_TEXTURE_TYPE _type, MARS_TEXTURE_DATA_TYPE _data_type) {
    gl_framebuffer_texture tex{};

    switch (_type) {
        case MARS_TEXTURE_TYPE_RGB8:
            tex.texture_type = GL_RGB;
            break;
        case MARS_TEXTURE_TYPE_RGBA8:
            tex.texture_type = GL_RGBA;
            break;
        case MARS_TEXTURE_TYPE_F_RGB16:
            tex.texture_type = GL_RGB16F;
            break;
        case MARS_TEXTURE_TYPE_F_RGBA16:
            tex.texture_type = GL_RGBA16F;
            break;
    }

    switch (_data_type) {
        case MARS_TEXTURE_DATA_TYPE_FLOAT:
            tex.texture_data_type = GL_FLOAT;
            break;
        case MARS_TEXTURE_DATA_TYPE_UNSIGNED_BYTE:
            tex.texture_data_type = GL_UNSIGNED_BYTE;
            break;
    }

    m_textures.push_back(tex);
}

void gl_framebuffer::create() {
    glGenFramebuffers(1, &m_frame_id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frame_id);

    m_framebuffers.resize(m_textures.size());
    glGenTextures((int)m_textures.size(), m_framebuffers.data());

    auto buffer_size = instance()->get_window()->size();

    std::vector<unsigned int> attachments;

    for (size_t i = 0; i < m_textures.size(); i ++) {
        glBindTexture(GL_TEXTURE_2D, m_framebuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, m_textures[i].texture_type, buffer_size.x(), buffer_size.y(), 0, GL_RGBA, m_textures[i].texture_data_type, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_framebuffers[i], 0);

        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    glDrawBuffers((int)attachments.size(), attachments.data());

    glGenRenderbuffers(1, &m_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, buffer_size.x(), buffer_size.y());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        mars_debug::debug::error("MARS - OpenGL - Framebuffer - Failed to create framebuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}