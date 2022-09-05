#ifndef _MATRIX4_
#define _MATRIX4_

#include "vector4.hpp"
#include "math.hpp"
#include "Debug/debug.hpp"

struct matrix3;

struct matrix4 {
    
    vector4 col0;
    vector4 col1;
    vector4 col2;
    vector4 col3;

    matrix4();
    matrix4(float value);
    matrix4(matrix3 value);
    matrix4(vector4 _col0, vector4 _col1, vector4 _col2, vector4 _col3);

    static matrix4 translate(vector3 translation);
    static matrix4 translate(matrix4 target, vector3 translation);
    static matrix4 scale(vector3 _scale);
    static matrix4 scale(matrix4 _target, vector3 _scale);
    static matrix4 look_at_lh(vector3 eye, vector3 center, vector3 up);
    static matrix4 look_at(vector3 eye, vector3 center, vector3 up);
    static matrix4 perspective_fov_lh(float fov, float width, float height, float z_near, float z_far);
    static matrix4 perspective_lh(float fov, float aspect, float z_near, float z_far);
    static matrix4 inverse(matrix4 value);
    static matrix4 inverse_transpose(matrix4 value);
    static matrix4 ortho(float left, float right, float bottom, float top, float z_near, float z_far);
    static matrix4 ortho_lh(float left, float right, float bottom, float top, float z_near, float z_far);
    static matrix4 from_quaternion(quaternion quat);
    static matrix4 create_from_axis_angle(vector3 axis, float angle);

    matrix4 operator*(const matrix4& right) const;
    matrix4 operator*(const float& right) const;
    vector4 operator*(const vector4& right) const;
    vector3 operator*(const vector3& right) const;
    matrix4 operator/=(const matrix4& right);
    matrix4 operator*=(const matrix4& right);
    matrix4 operator=(const matrix4& right);
    vector4& operator[](int index);
};


#endif