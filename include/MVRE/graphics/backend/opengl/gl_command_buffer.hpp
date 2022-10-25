#ifndef __MVRE__GL__COMMAND__BUFFER__
#define __MVRE__GL__COMMAND__BUFFER__

#include <MVRE/graphics/backend/base/base_command_buffer.hpp>
#include <glad/glad.h>

namespace mvre_graphics_opengl {

    class gl_command_buffer : public mvre_graphics_base::base_command_buffer {
    public:
        void draw(int count, int size) override {
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    };
}

#endif