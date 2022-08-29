#ifndef _VECTOR3_
#define _VECTOR3_

#include "vector2.hpp"
#include "math.hpp"
#include "Graphics/Vulkan/vtypes.hpp"

struct vector3 {
    
    float x;
    float y;
    float z;

    vector3();
    vector3(float _x, float _y, float _z);

    static vector3 zero();
    static vector3 one();
    static vector3 up();
    static vector3 forward();
    
    vector2 xy();
    vector3 get_safe_normal(float tolerance = SMALL_NUMBER);
    float length();
    float length_squared();

    static vector3 move_towards(vector3 current, vector3 target, float max_distance_delta);
    static vector3 rotate_around(vector3 position, vector3 target, vector3 up, float angle);
    static float distance(vector3 origin, vector3 target);
    static float dot(vector3 Left, vector3 Right);
    float angle_between(vector3 Left, vector3 Right);
    vector3 projection(vector3 Left, vector3 Right);

    bool operator==(const vector3& other) const;
    bool operator!=(const vector3& other) const;

    vector3 operator+(const vector3& right) const;
    vector3 operator+(const vector2& right) const;
    vector3 operator-(const vector3& right) const;
    vector3 operator*(const vector3& right) const;
    vector3 operator*(const float& right) const;
    vector3 operator/(const vector3& right) const;
    vector3 operator/(const float& right) const;

    vector3& operator+=(const vector3& right);
    vector3& operator+=(const vector2& right);
    vector3& operator-=(const vector3& right);

    float& operator[](int index);

    size_t operator()(const vector3& v) const;

    ~vector3() = default;

    static VertexInputDescription get_description();
};



#endif