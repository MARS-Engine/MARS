#ifndef __LIGHT__MANAGER__
#define __LIGHT__MANAGER__

#include "Math/vector4.hpp"
#include "Graphics/shader_data.hpp"

static const size_t MAX_POINT_LIGHT = 2;

struct point_light {
    vector4 position;
    vector4 color;
};

struct sun {
    vector4 direction;
    vector4 color;
    vector4 ambient;
};

struct point_light_data {
    vector4 ambient = vector4(1.0f, 1.0f, 1.0f, .02f);
    point_light lights[MAX_POINT_LIGHT];
    int light_size = 0;
};

struct global_light_shader_data {
    sun g_sun;
    vector3 camPos;
};

class light_manager {
private:
    static global_light_shader_data _shader_data;
public:
    static sun g_sun;
    static point_light_data pl_data;
    static void add_point_light(point_light light);
    static void generate_shader_uniform(shader_data* data);
    static void update(vector3 cam);
    static void update_shader_uniform(shader_data* data);
};

#endif