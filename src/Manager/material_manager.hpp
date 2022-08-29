#ifndef __MATERIAL__MANAGER__
#define __MATERIAL__MANAGER__

#include "Graphics/material.hpp"
#include <vector>

using namespace std;

class material_manager {
public:
    static vector<material*> materials;

    static material* get_material(const string& name);
};

#endif