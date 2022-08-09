#include "Quaternion.hpp"
#include "Math.hpp"
#include "Matrix3.hpp"
#include <math.h>

Quaternion::Quaternion() {
    Quat = Vector4(0, 0, 0, 1);
}

Quaternion::Quaternion(Vector4 quat) {
    Quat = quat;
}

Quaternion::Quaternion(Vector3 Xyz, float W) {
    Quat = Vector4(Xyz, W);
}


float Quaternion::Length() {
    return sqrt(Quat.w * Quat.w + Quat.Xyz().LengthSquared());
}

Vector3 Quaternion::ToEuler() {

    Vector3 Euler;

    float sinr_cosp = 2 * (Quat.w * Quat.x + Quat.y * Quat.z);
    float consr_cosp = 1 - 2 * (Quat.x * Quat.x + Quat.y * Quat.y);
    Euler.x = atan2(sinr_cosp, consr_cosp);

    float sinp = 2 * (Quat.w * Quat.y - Quat.z * Quat.x);
    if (abs(sinp) >= 1)
        Euler.y = copysign(M_PI / 2, sinp);
    else
        Euler.y = asin(sinp);

    float siny_cosp = 2 * (Quat.w * Quat.z + Quat.x * Quat.y);
    float cosy_cosp = 1 - 2 * (Quat.y * Quat.y + Quat.z * Quat.z);
    Euler.z = atan2(siny_cosp, cosy_cosp);

    return Euler;
}

Quaternion Quaternion::EulerToQuaternion(Vector3 Euler) {

    float cy = cos(Euler.z * 0.5);
    float sy = sin(Euler.z * 0.5);
    float cp = cos(Euler.y * 0.5);
    float sp = sin(Euler.y * 0.5);
    float cr = cos(Euler.x * 0.5);
    float sr = sin(Euler.x * 0.5);

    Vector4 q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return Quaternion(q);
}

Quaternion CreateFromAxisAngle(Vector3 axis, float angle) {
    float halfAngle = angle * .5f;
    float s = sin(halfAngle);
    Quaternion q;
    q.Quat.x = axis.x * s;
    q.Quat.y = axis.y * s;
    q.Quat.z = axis.z * s;
    q.Quat.w = cos(halfAngle);
    return q;
}

Quaternion Quaternion::LookRotation(Vector3 Source, Vector3 Destination) {

    Vector3 forwardVector = Normalize(Destination - Source);

    float dot = Dot(Vector3(0, 0, 1), forwardVector);

    if (abs(dot - (-1.0f)) < 0.000001f)
        return Quaternion(Vector4(0.0f , 1.0f, 0.0f, M_PI));
    if (abs(dot - (1.0f)) < 0.000001f)
        return Quaternion();

    float rotAngle = acos(dot);
    Vector3 rotAxis = Cross(Vector3(0, 0, 1), forwardVector);
    rotAxis = Normalize(rotAxis);
    return CreateFromAxisAngle(rotAxis, rotAngle);
}

Quaternion Quaternion::SlerpNotNormalized(Quaternion Left, Quaternion Right, float Slerp) {

    float RawCosom = Left.Quat.x * Right.Quat.x + Left.Quat.y * Right.Quat.y + Left.Quat.z * Right.Quat.z + Left.Quat.w * Right.Quat.w;
    float Cosom = RawCosom >= 0.f ? RawCosom : -RawCosom;

    float Scale0, Scale1;

    if (Cosom < 0.9999f) {

        float Omega = acos(Cosom);
        float InvSin = 1.f/sin(Omega);
        Scale0 = sin((1.f - Slerp) * Omega) * InvSin;
        Scale1 = sin(Slerp * Omega) * InvSin;
    }
    else {
        Scale0 = 1.0f - Slerp;
        Scale1 = Slerp;
    }

    Scale1 = RawCosom >= 0.f ? Scale1 : -Scale1;

    return Quaternion(Vector4(Scale0 * Left.Quat.x + Scale1 * Right.Quat.x,
                              Scale0 * Left.Quat.y + Scale1 * Right.Quat.y,
                              Scale0 * Left.Quat.z + Scale1 * Right.Quat.z,
                              Scale0 * Left.Quat.w + Scale1 * Right.Quat.w));
}

Quaternion Quaternion::Slerp(Quaternion Left, Quaternion Right, float Slerp) {
    return Normalize(SlerpNotNormalized(Left, Right, Slerp));
}

Quaternion Quaternion::lerp(Quaternion Left, Quaternion Right, float Time) {

    return Left * (1.0f - Time) + (Right * Time);
}

Quaternion Quaternion::FromAxisAngle(Vector3 Axis, float Angle) {
    if (Axis.LengthSquared() == 0.0f)
        return Quaternion();

    Angle *= 0.5f;
    return Normalize(Quaternion(Normalize(Axis) * sin(Angle), cos(Angle)));
}

Vector4 Quaternion::ToAxisAngle(Quaternion quat) {
    
    Quaternion q = quat;
    if (abs(quat.Quat.w) > 1.0f)
        q = Normalize(q);

    Vector4 Result = Vector4(0.0f, 0.0f, 0.0f, 2.0f * acos(q.Quat.w));
    
    float den = sqrt(1.0f - (q.Quat.w * q.Quat.w));

    if (den > 0.0001f)
        Result.Xyz(q.Quat.Xyz() / den);
    else
        Result.Xyz(Vector3(1.0f, 0.0f, 0.0f));

    return Result;
}

Quaternion Quaternion::operator*(float right) {

    return Quaternion(Vector4(
            Quat.x * right,
            Quat.y * right,
            Quat.z * right,
            Quat.w * right
    )); 
}

Quaternion Quaternion::operator*(Quaternion right) {

    return Quaternion(Vector4(
            Quat.w * right.Quat.x + Quat.x * right.Quat.w + Quat.y * right.Quat.z - Quat.z * right.Quat.y,
            Quat.w * right.Quat.y + Quat.y * right.Quat.w + Quat.z * right.Quat.x - Quat.x * right.Quat.z,
            Quat.w * right.Quat.z + Quat.z * right.Quat.w + Quat.x * right.Quat.y - Quat.y * right.Quat.x,
            Quat.w * right.Quat.w - Quat.x * right.Quat.x - Quat.y * right.Quat.y - Quat.z * right.Quat.z
    ));
}

Quaternion Quaternion::operator+(Quaternion right) {

    return Quaternion(Vector4(
            Quat.x + right.Quat.x,
            Quat.y + right.Quat.y,
            Quat.z + right.Quat.z,
            Quat.w + right.Quat.w));
}

Vector3 Quaternion::operator*(Vector3 right) {

    Vector3 Xyz = Quat.Xyz();
    Vector3 Uv = Cross(Xyz, right);
    Vector3 Uuv = Cross(Xyz, Uv);

    return right + ((Uv * Quat.w) + Uuv) * 2.0f;
}