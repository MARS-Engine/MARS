#include "MaterialManager.hpp"

vector<Material*> MaterialManager::materials;

Material* MaterialManager::GetMaterial(const string& name) {
    for (auto mat : materials)
        if (mat->name == name)
            return mat;
    auto mat = new Material(name);
    return mat;
}