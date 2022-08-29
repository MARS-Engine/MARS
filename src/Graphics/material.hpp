#ifndef __MATERIAL__
#define __MATERIAL__

#include <string>
#include "Math/vector4.hpp"
#include "shader.hpp"
#include "texture.hpp"

using namespace std;

struct material_data {
    vector4 diffuse;
};

/// Generic class with values that can be used by renderers and is used for sorting
struct material {
public:
    string name;
    bool enable_transparency;
    material_data data;
    shader* mat_shader;
    vector<texture*> textures;

    explicit material(const string& _name);
};

#endif