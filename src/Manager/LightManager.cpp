#include "LightManager.hpp"

PointLightData LightManager::pointLightData;
Sun LightManager::sun{};

void LightManager::AddPointLight(PointLight light) {
    pointLightData.lights[pointLightData.lightSize++] = light;
}
