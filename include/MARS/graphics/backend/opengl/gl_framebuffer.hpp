#ifndef MARS_GL_FRAMEBUFFER_
#define MARS_GL_FRAMEBUFFER_

#include <MARS/graphics/backend/template/framebuffer.hpp>
#include <glad/glad.h>

namespace mars_graphics {
    struct gl_framebuffer_texture {
        int texture_type;
        int texture_data_type;
    };

    class gl_framebuffer : public framebuffer {
    private:
        unsigned int m_frame_id;
        unsigned int m_depth;
        std::vector<gl_framebuffer_texture> m_textures;
        std::vector<unsigned int> m_framebuffers;
    public:
        using framebuffer::framebuffer;

        void add_texture(MARS_TEXTURE_TYPE _type, MARS_TEXTURE_DATA_TYPE _data_type) override;
        void create() override;

        void bind() override { glBindFramebuffer(GL_FRAMEBUFFER, m_frame_id); }

        void bind_textures() override {
            for (size_t i = 0; i < m_framebuffers.size(); i++) {
                glActiveTexture(GL_TEXTURE0 + i + 2);
                glBindTexture(GL_TEXTURE_2D, m_framebuffers[i]);
            }
        }

        void unbind() override { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    };
}

#endif