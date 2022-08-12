#include "Shader.hpp"
#include "VEngine.hpp"
#include "Vulkan/VShader.hpp"

Shader::Shader(VEngine* _engine) {
    engine = _engine;
}
void Shader::LoadShader(const string& location) {
    shaderPath = location;
    vShader = new VShader(engine->device);
    vShader->LoadShader(location);
}