#ifndef __SHADER__TYPES__
#define __SHADER__TYPES__

#include "Math/Matrix4.hpp"

struct ShaderModel {
    Matrix4 mvp;
    Matrix4 model;
};

#endif