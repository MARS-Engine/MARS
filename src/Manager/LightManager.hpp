#ifndef __LIGHTMANAGER__
#define __LIGHTMANAGER__

#include "Math/Vector4.hpp"

static const size_t MAX_POINT_LIGHT = 2;

struct PointLight {
    Vector4 pos;
    Vector4 color;
};

struct Sun {
    Vector4 direction;
    Vector4 color;
    Vector4 ambient;
};

struct PointLightData {
    Vector4 ambient = Vector4(1.0f, 1.0f, 1.0f, .02f);
    PointLight lights[MAX_POINT_LIGHT];
    int lightSize = 0;
};

class LightManager {
public:
    static Sun sun;
    static PointLightData pointLightData;
    static void AddPointLight(PointLight light);
};

#endif