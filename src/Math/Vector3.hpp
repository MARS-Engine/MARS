#ifndef _VECTOR3_
#define _VECTOR3_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Vector2.hpp"
#include "Math.hpp"

struct Vector3 {
    
    float x;
    float y;
    float z;

    Vector3();
    Vector3(float _x, float _y, float _z);

    static Vector3 Zero();
    static Vector3 Up();
    static Vector3 Forward();
    
    Vector2 Xy();
    Vector3 GetSafeNormal(float Tolerance = SMALL_NUMBER);
    float Length();
    float LengthSquared();

    static Vector3 MoveTowards(Vector3 Current, Vector3 Target, float MaxDistanceDelta);
    static Vector3 RotateAround(Vector3 Position, Vector3 Target, Vector3 Up, float Angle);
    
    bool operator==(const Vector3& other) const;
    bool operator!=(const Vector3& other) const;

    Vector3 operator+(const Vector3& right) const;
    Vector3 operator+(const Vector2& right) const;
    Vector3 operator-(const Vector3& right) const;
    Vector3 operator*(const Vector3& right) const;
    Vector3 operator*(const float& right) const;
    Vector3 operator/(const Vector3& right) const;
    Vector3 operator/(const float& right) const;

    Vector3& operator+=(const Vector3& right);
    Vector3& operator+=(const Vector2& right);
    Vector3& operator-=(const Vector3& right);

    float& operator[](int Index);
};



#endif