#ifndef __MATERIAL__
#define __MATERIAL__

#include <string>
#include "Math/Vector4.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

using namespace std;

struct MaterialData {
    Vector4 diffuse;
};

/// Generic class with values that can be used by renderers and is used for sorting
struct Material {
public:
    string name;
    bool enableTransparency;
    MaterialData data;
    Shader* shader;
    vector<Texture*> textures;

    explicit Material(const string& name);
};

#endif