#ifndef _VECTOR4_
#define _VECTOR4_

#include "vector3.hpp"

struct vector4 {
    
    float x;
    float y;
    float z;
    float w;

    vector4();
    vector4(vector2 xy, float _z, float _w);
    vector4(vector2 xy, vector2 zw);
    vector4(vector3 xyz);
    vector4(vector3 xyz, float _w);
    vector4(float value);
    vector4(float _x, float _y, float _z);
    vector4(float _x, float _y, float _z, float _w);

    void xyz(vector3 xyz);
    vector3 xyz() const;
    vector2 xy() const;

    vector4 operator+(const vector4& right) const;
    vector4 operator-(const vector4& right) const;
    vector4 operator*(const float& right) const;
    vector4 operator*(const vector4& right) const;
    vector4& operator+=(const vector4& right);
    vector4& operator*=(const float& right);
    float& operator[](int index);
};


#endif