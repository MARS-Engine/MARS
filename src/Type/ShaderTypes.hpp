#ifndef __SHADERTYPES__
#define __SHADERTYPES__

#include "Math/Matrix4.hpp"

struct ShaderModel {
    Matrix4 mvp;
    Matrix4 model;
};

#endif