#ifndef _TRIANGLE_
#define _TRIANGLE_

#include "Vector3.hpp"
#include "Matrix4.hpp"

struct Triangle {
    
    Vector3 Pos1; 
    Vector3 Pos2; 
    Vector3 Pos3;
    Vector3 Edge1;
    Vector3 Edge2;

    Triangle(Vector3 pos1, Vector3 pos2, Vector3 pos3);

    Triangle operator+(const Vector3& other) const;
    Triangle operator*(const Matrix4& other) const;
};


#endif