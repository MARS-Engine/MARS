#ifndef _MATRIX3_
#define _MATRIX3_

#include "vector3.hpp"
#include "matrix4.hpp"
#include "quaternion.hpp"

struct matrix3 {
public:
    vector3 col0;
    vector3 col1;
    vector3 col2;

    matrix3();
    matrix3(matrix4 v);
};


#endif