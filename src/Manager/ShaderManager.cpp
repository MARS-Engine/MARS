#include "ShaderManager.hpp"
#include "Graphics/Shader.hpp"

vector<Shader*> ShaderManager::shaders;

Shader *ShaderManager::GetShader(const std::string& path, VEngine *engine) {
    for (auto shader : shaders)
        if (shader->shaderPath == path)
            return shader;

    auto shader = new Shader(engine);
    shader->LoadShader(path);
    shaders.push_back(shader);
    return shader;
}

void ShaderManager::Clean() {
    shaders.clear();
}