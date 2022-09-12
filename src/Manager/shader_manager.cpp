#include "shader_manager.hpp"
#include "Graphics/shader.hpp"

std::vector<shader*> shader_manager::shaders;

shader *shader_manager::get_shader(const std::string& path, vengine *engine) {
    for (auto shader : shaders)
        if (shader->location == path)
            return shader;

    auto new_shader = new shader(engine);
    new_shader->load_shader(path);
    shaders.push_back(new_shader);
    return new_shader;
}

void shader_manager::clean() {
    shaders.clear();
}