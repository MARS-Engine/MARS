#include "material_manager.hpp"

vector<material*> material_manager::materials;

material* material_manager::get_material(const string& name) {
    for (auto mat : materials)
        if (mat->name == name)
            return mat;
    auto mat = new material(name);
    return mat;
}