#ifndef __MATERIALMANAGER__
#define __MATERIALMANAGER__

#include "Graphics/Material.hpp"
#include <vector>

using namespace std;

class MaterialManager {
public:
    static vector<Material*> materials;

    static Material* GetMaterial(const string& name);
};

#endif