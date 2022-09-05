#ifndef _TRIANGLE_
#define _TRIANGLE_

#include "vector3.hpp"
#include "matrix4.hpp"

struct triangle {
    
    vector3 pos1;
    vector3 pos2;
    vector3 pos3;
    vector3 edge1;
    vector3 edge2;

    triangle(vector3 _pos1, vector3 _pos2, vector3 _pos3);

    triangle operator+(const vector3& other) const;
    triangle operator*(const matrix4& other) const;
};


#endif