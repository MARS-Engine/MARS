#include "shader.hpp"
#include "vengine.hpp"
#include "Vulkan/vshader.hpp"

shader::shader(vengine* _engine) {
    engine = _engine;
}
void shader::load_shader(const string& _location) {
    location = _location;
    base_shader = new VShader(engine->device);
    base_shader->load_shader(_location);
}