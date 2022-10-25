#include <MVRE/graphics/shader.hpp>
#include <MVRE/graphics/backend/opengl/gl_shader.hpp>

using namespace mvre_graphics;
using namespace mvre_resources;
using namespace mvre_graphics_opengl;

bool shader::load_resource(const std::string& _path) {
    bool success;
    
    switch (instance->get_backend_type()) {
        case MVRE_INSTANCE_TYPE_VULKAN:
            break;
        case MVRE_INSTANCE_TYPE_OPENGL:
            gl_shader* temp_shader;
            success = resource_manager::load_resource<gl_shader>(_path, temp_shader, instance);
            raw_shader = temp_shader;
            break;
    }

    return success;
}