#include <MVRE/graphics/shader_input.hpp>
#include <MVRE/graphics/backend/opengl/gl_shader_input.hpp>

using namespace mvre_graphics;

shader_input::shader_input(engine_instance* _instance) {
    instance = _instance;

    switch (instance->get_backend_type()) {
        case MVRE_INSTANCE_TYPE_VULKAN:
            break;
        case MVRE_INSTANCE_TYPE_OPENGL:
            raw_shader_input = new mvre_graphics_opengl::gl_shader_input();
            break;
    }    
}