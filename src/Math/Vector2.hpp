#ifndef _VECTOR2_
#define _VECTOR2_

#include <vector>

using namespace std;

struct Vector2 {
    float x, y;
    //temporary (most likely final) vulkan buffer alignment fix
    union  { float _pad0; };
    union  { float _pad1; };

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