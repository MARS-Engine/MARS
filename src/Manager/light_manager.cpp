#include "light_manager.hpp"

point_light_data light_manager::pl_data;
sun light_manager::g_sun{};
global_light_shader_data light_manager::_shader_data{};

void light_manager::add_point_light(point_light light) {
    pl_data.lights[pl_data.light_size++] = light;
}

void light_manager::generate_shader_uniform(shader_data* data) {
    data->get_uniform("__GLOBAL_LIGHT")->generate(sizeof(global_light_shader_data));
}

void light_manager::update(vector3 cam) {
    _shader_data.camPos = cam;
    _shader_data.g_sun = g_sun;
}

void light_manager::update_shader_uniform(shader_data* data) {
    data->get_uniform("__GLOBAL_LIGHT")->update(&_shader_data);
}