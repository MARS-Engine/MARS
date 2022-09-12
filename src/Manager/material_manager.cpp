#include "material_manager.hpp"

std::vector<material*> material_manager::materials;

material* material_manager::get_material(const std::string& _name, vengine* _engine) {
    for (auto mat : materials)
        if (mat->name == _name)
            return mat;
    auto mat = new material(_engine, _name);
    materials.push_back(mat);
    return mat;
}