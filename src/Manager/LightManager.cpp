#include "LightManager.hpp"

PointLightData LightManager::pointLightData;
Sun LightManager::sun{};

void LightManager::AddPointLight(PointLight light) {
    pointLightData.lights[pointLightData.lightSize++] = light;
}

void LightManager::GenerateShaderUniform(ShaderData* data) {
    data->GetUniform("__GLOBAL_LIGHT")->Generate(sizeof(sun));
}

void LightManager::UpdateShaderUniform(ShaderData* data) {
    data->GetUniform("__GLOBAL_LIGHT")->Update(&sun);
}