#ifndef _VECTOR2_
#define _VECTOR2_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

using namespace std;

struct Vector2 {
    
    float x, y;

    Vector2();
    Vector2(float x, float y);

    static float AngleBetween(Vector2 Right, Vector2 Left);

    bool operator==(const Vector2& other) const;
    bool operator!=(const Vector2& other) const;
    Vector2 operator+(const Vector2& right) const;
    Vector2 operator+(const float& right) const;
    Vector2 operator-(const Vector2& right) const;
    Vector2 operator-(const float& right) const;
    Vector2 operator*(const Vector2& right) const;
    Vector2 operator*(const float& right) const;
    Vector2 operator/(const Vector2& right) const;
    Vector2 operator/(const float& right) const;
    bool operator>(const Vector2& right) const;
    bool operator<(const Vector2& right) const;

    size_t operator()(const Vector2& v) const;

};


#endif