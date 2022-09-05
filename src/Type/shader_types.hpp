#ifndef __SHADER__TYPES__
#define __SHADER__TYPES__

#include "Math/matrix4.hpp"

struct shader_model {
    matrix4 mvp;
    matrix4 model;
};

#endif