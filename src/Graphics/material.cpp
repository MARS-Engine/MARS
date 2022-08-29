#include "material.hpp"

material::material(const string& _name) {
    name = _name;
    enable_transparency = false;
    data = {};
}