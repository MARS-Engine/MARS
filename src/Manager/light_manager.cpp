#include "light_manager.hpp"

point_light_data light_manager::pl_data;
sun light_manager::g_sun{};

void light_manager::add_point_light(point_light light) {
    pl_data.lights[pl_data.light_size++] = light;
}

void light_manager::generate_shader_uniform(shader_data* data) {
    data->get_uniform("__GLOBAL_LIGHT")->generate(sizeof(g_sun));
}

void light_manager::update_shader_uniform(shader_data* data) {
    data->get_uniform("__GLOBAL_LIGHT")->update(&g_sun);
}