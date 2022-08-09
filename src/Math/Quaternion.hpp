#ifndef _QUATERNION_
#define _QUATERNION_

#include "Matrix4.hpp"

struct Quaternion {
public:
    Vector4 Quat;

    Quaternion();
    Quaternion(Vector4 quat);
    Quaternion(Vector3 Xyz, float W);
    float Length();
    Vector3 ToEuler();

    static Quaternion EulerToQuaternion(Vector3 Euler);
    static Quaternion lerp(Quaternion Left, Quaternion Right, float Time);
    static Quaternion SlerpNotNormalized(Quaternion Left, Quaternion Right, float Slerp);
    static Quaternion Slerp(Quaternion Left, Quaternion Right, float Slerp);
    static Quaternion LookRotation(Vector3 Target, Vector3 UpDirection);
    static Quaternion FromAxisAngle(Vector3 Axis, float Angle);
    static Vector4 ToAxisAngle(Quaternion quat);
    static Matrix4 toMatrix4(Quaternion quat);

    Quaternion operator*(float right);
    Quaternion operator*(Quaternion right);
    Quaternion operator+(Quaternion right);
    Vector3 operator*(Vector3 right);
};

#endif