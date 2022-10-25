#include <MVRE/graphics/command_buffer.hpp>
#include <MVRE/graphics/backend/opengl/gl_command_buffer.hpp>

using namespace mvre_graphics;

void command_buffer::load() {
    switch (instance->get_backend_type()) {
        case MVRE_INSTANCE_TYPE_VULKAN:
            break;
        case MVRE_INSTANCE_TYPE_OPENGL:
            raw_command_buffer = new mvre_graphics_opengl::gl_command_buffer();
            break;
    }
}