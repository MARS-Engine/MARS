#ifndef __MVRE__GL__TEXTURE__
#define __MVRE__GL__TEXTURE__

#include <MVRE/graphics/backend/template/texture.hpp>
#include <glad/glad.h>

namespace mvre_graphics {

    class gl_texture : public texture{
    private:
        unsigned int m_texture_id = 0;
    public:
        using texture::texture;

        inline void bind() override { glBindTexture(GL_TEXTURE_2D, m_texture_id); }
        inline void unbind() override { glBindTexture(GL_TEXTURE_2D, 0);  }

        bool load_resource(const std::string &_texture_path) override;
        void clean() override;
    };
}

#endif