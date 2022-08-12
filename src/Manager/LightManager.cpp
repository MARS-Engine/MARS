#include "LightManager.hpp"

PointLightData LightManager::pointLightData;

void LightManager::AddPointLight(PointLight light) {
    pointLightData.lights[pointLightData.lightSize++] = light;
}
