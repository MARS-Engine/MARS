#ifndef MARS_GL_COMMAND_BUFFER_
#define MARS_GL_COMMAND_BUFFER_

#include <MARS/graphics/backend/template/command_buffer.hpp>
#include <glad/glad.h>

namespace mars_graphics {

    class gl_command_buffer : public command_buffer {
    public:
        using command_buffer::command_buffer;

        void draw(int first, size_t _count) override { glDrawArrays(GL_TRIANGLES, first, _count); }
        void draw_indexed(size_t _indices) override { glDrawElements(GL_TRIANGLES, _indices, GL_UNSIGNED_INT, nullptr); }
        void draw_instanced(size_t _indices, size_t _instances) override { glDrawElementsInstanced(GL_TRIANGLES, _indices, GL_UNSIGNED_INT, 0, _instances); }
    };
}

#endif