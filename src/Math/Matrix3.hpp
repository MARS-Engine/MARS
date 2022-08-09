#ifndef _MATRIX3_
#define _MATRIX3_

#include "Vector3.hpp"
#include "Matrix4.hpp"
#include "Quaternion.hpp"

struct Matrix3 {
public:
    Vector3 col0;
    Vector3 col1;
    Vector3 col2;

    Matrix3(Matrix4 v);
};


#endif