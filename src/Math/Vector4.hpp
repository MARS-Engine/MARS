#ifndef _VECTOR4_
#define _VECTOR4_

#include "Vector3.hpp"

struct Vector4 {
    
    float x;
    float y;
    float z;
    float w;

    Vector4();
    Vector4(Vector2 Xy, float _z, float _w);
    Vector4(Vector2 Xy, Vector2 Zw);
    Vector4(Vector3 Xyz);
    Vector4(Vector3 Xyz, float _w);
    Vector4(float value);
    Vector4(float _x, float _y, float _z);
    Vector4(float _x, float _y, float _z, float _w);

    void Xyz(Vector3 Xyz);
    Vector3 Xyz() const;
    Vector2 Xy() const;

    Vector4 operator+(const Vector4& right) const;
    Vector4 operator-(const Vector4& right) const;
    Vector4 operator*(const float& right) const;
    Vector4 operator*(const Vector4& right) const;
    Vector4& operator+=(const Vector4& right);
    Vector4& operator*=(const float& right);
};


#endif