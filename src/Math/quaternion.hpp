#ifndef _QUATERNION_
#define _QUATERNION_

#include "matrix4.hpp"

struct quaternion {
public:
    vector4 value;

    quaternion();
    quaternion(vector4 _value);
    quaternion(vector3 xyz, float W);

    float length();
    vector3 to_euler();

    static float pitch(quaternion quat);
    static float yaw(quaternion quat);
    static float roll(quaternion quat);

    static quaternion euler_to_quaternion(vector3 euler);
    static quaternion lerp(quaternion left, quaternion right, float time);
    static quaternion slerp_not_normalized(quaternion left, quaternion right, float slerp);
    static quaternion slerp(quaternion left, quaternion right, float slerp);
    static quaternion look_rotation(vector3 target, vector3 up_direction);
    static quaternion from_axis_angle(vector3 axis, float angle);
    static vector4 to_axis_angle(quaternion quat);
    static matrix4 to_matrix4(quaternion quat);

    quaternion operator*(float right);
    quaternion operator*(quaternion right);
    quaternion operator+(quaternion right);
    bool operator==(const quaternion& right) const;
    bool operator!=(const quaternion& right) const;
    vector3 operator*(vector3 right);
};

#endif