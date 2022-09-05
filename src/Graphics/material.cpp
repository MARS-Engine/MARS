#include "material.hpp"
#include "Manager/shader_manager.hpp"

material::material(vengine* _vengine, const string& _name) {
    name = _name;
    enable_transparency = false;
    _engine = _vengine;
    data = {};
}

void material::load_shader(const string& _shader_location) {
    _shader = shader_manager::get_shader(_shader_location, _engine);
}

void material::add_texture(const string& _name, texture* _texture) {
    _textures.insert(pair<string, texture*>(_name, _texture));
}

void material::apply_values(shader_data* _data) {
    for (auto p : _textures)
        _data->get_uniform(p.first)->setTexture(p.second);
}