#ifndef __MATERIAL__
#define __MATERIAL__

#include <string>
#include "Math/vector4.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "shader_data.hpp"

using namespace std;

struct material_data {
    vector4 diffuse;
};

/**
 * @brief Generic class with values that can be used by renderers and is used for sorting
 */
class material {
private:
    shader* _shader = nullptr;
    map<string, texture*> _textures;
    vengine* _engine  = nullptr;
public:
    string name;
    bool enable_transparency;
    material_data data;

    inline shader* get_shader() { return _shader; }

    explicit material(vengine* _vengine, const string& _name);

    void load_shader(const string& _shader_location);
    void add_texture(const string& _name, texture* _texture);

    void apply_values(shader_data* _data);
};

#endif