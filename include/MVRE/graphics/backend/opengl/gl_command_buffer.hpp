#ifndef MVRE_GL_COMMAND_BUFFER_
#define MVRE_GL_COMMAND_BUFFER_

#include <MVRE/graphics/backend/template/command_buffer.hpp>
#include <glad/glad.h>

namespace mvre_graphics {

    class gl_command_buffer : public command_buffer {
    public:
        using command_buffer::command_buffer;

        void draw(int first, size_t _count) override { glDrawArrays(GL_TRIANGLES, first, _count); }
        void draw_indexed(size_t _indices) override { glDrawElements(GL_TRIANGLES, _indices, GL_UNSIGNED_INT, nullptr); }
    };
}

#endif