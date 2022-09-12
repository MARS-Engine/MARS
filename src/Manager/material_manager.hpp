#ifndef __MATERIAL__MANAGER__
#define __MATERIAL__MANAGER__

#include "Graphics/material.hpp"
#include <vector>



class material_manager {
public:
    static std::vector<material*> materials;

    static material* get_material(const std::string& _name, vengine* _engine);
};

#endif