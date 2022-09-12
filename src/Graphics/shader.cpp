#include "shader.hpp"
#include "vengine.hpp"
#include "Vulkan/vshader.hpp"

std::string shader::render_type_to_path() {
    switch (engine->type) {
        case MVRE_RENDERER_SIMPLE:
            return "Simple/";
        case MVRE_RENDERER_DEFERRED:
            return "Deferred/";
    }
    return "";
}

shader::shader(vengine* _engine) {
    engine = _engine;
}
void shader::load_shader(const std::string& _location) {
    location = _location;
    base_shader = new VShader(engine->device);
    base_shader->load_shader(render_type_to_path() + _location);
}