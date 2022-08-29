#ifndef _VECTOR2_
#define _VECTOR2_

#include <vector>

using namespace std;

struct vector2 {
    float x, y;
    //temporary (most likely final) vulkan buffer alignment fix
    union  { float _pad0; };
    union  { float _pad1; };

    vector2();
    vector2(float x, float y);

    static float angle_between(vector2 Right, vector2 Left);

    bool operator==(const vector2& other) const;
    bool operator!=(const vector2& other) const;
    vector2 operator+(const vector2& right) const;
    vector2 operator+(const float& right) const;
    vector2 operator-(const vector2& right) const;
    vector2 operator-(const float& right) const;
    vector2 operator*(const vector2& right) const;
    vector2 operator*(const float& right) const;
    vector2 operator/(const vector2& right) const;
    vector2 operator/(const float& right) const;
    bool operator>(const vector2& right) const;
    bool operator<(const vector2& right) const;

    size_t operator()(const vector2& v) const;

};


#endif