#ifndef MARS_GL_PIPELINE_
#define MARS_GL_PIPELINE_

#include <MARS/graphics/backend/template/pipeline.hpp>
#include <glad/glad.h>

namespace mars_graphics {

    class gl_pipeline : public pipeline {
    private:

    public:
        using pipeline::pipeline;

        void bind() override {
            glViewport(m_viewport.position.x(), m_viewport.position.y(), m_viewport.size.x(), m_viewport.size.y());
            m_shader->bind();
        }
    };
}

#endif