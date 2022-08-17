#include "Quaternion.hpp"
#include "Math.hpp"
#include "Matrix3.hpp"
#include <math.h>

Quaternion::Quaternion() {
    quat = Vector4(0, 0, 0, 1);
}

Quaternion::Quaternion(Vector4 _quat) {
    quat = _quat;
}

Quaternion::Quaternion(Vector3 Xyz, float W) {
    quat = Vector4(Xyz, W);
}


float Quaternion::Length() {
    return sqrt(quat.w * quat.w + quat.Xyz().LengthSquared());
}

Vector3 Quaternion::ToEuler() {
    return {Pitch(*this), Yaw(*this), Roll(*this)}; //Note sure if this works, euler should be avoided regardless
}

float Quaternion::Pitch(Quaternion _quat) {
    float y = 2.0f * (_quat.quat.y * _quat.quat.z + _quat.quat.w * _quat.quat.x);
    float x = _quat.quat.w * _quat.quat.w - _quat.quat.x * _quat.quat.x - _quat.quat.y * _quat.quat.y + _quat.quat.z * _quat.quat.z;
    if (Equals(x, 0.0f, numeric_limits<float>::epsilon()) && Equals(y, 0.0f, numeric_limits<float>::epsilon()))
        return 2.0f * atan2f(_quat.quat.x, _quat.quat.w);
    return atan2f(y, x);
}

float Quaternion::Yaw(Quaternion _quat) {
    return asin(clamp(-2.0f * (_quat.quat.x * _quat.quat.z - _quat.quat.w * _quat.quat.y), -1.0f, 1.0f));
}

float Quaternion::Roll(Quaternion _quat) {
    atan2(2.0f * (_quat.quat.x * _quat.quat.y + _quat.quat.w * _quat.quat.z), _quat.quat.w * _quat.quat.w + _quat.quat.x * _quat.quat.x - _quat.quat.y * _quat.quat.y - _quat.quat.z * _quat.quat.z);
}

Quaternion Quaternion::EulerToQuaternion(Vector3 Euler) {

    Vector3 c = Vector3(cosf(Euler.x * deg2rad * 0.5f), cosf(Euler.y * deg2rad * 0.5f), cosf(Euler.z * deg2rad * 0.5f));
    Vector3 s = Vector3(sinf(Euler.x * deg2rad * 0.5f), sinf(Euler.y * deg2rad * 0.5f), sinf(Euler.z * deg2rad * 0.5f));

    Vector4 q;
    q.w = c.x * c.y * c.z + s.x * s.y * s.z;
    q.x = s.x * c.y * c.z - c.x * s.y * s.z;
    q.y = c.x * s.y * c.z + s.x * c.y * s.z;
    q.z = c.x * c.y * s.z - s.x * s.y * c.z;

    return Quaternion(q);
}

Quaternion CreateFromAxisAngle(Vector3 axis, float angle) {
    float halfAngle = angle * .5f;
    float s = sin(halfAngle);
    Quaternion q;
    q.quat.x = axis.x * s;
    q.quat.y = axis.y * s;
    q.quat.z = axis.z * s;
    q.quat.w = cos(halfAngle);
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

    float RawCosom = Left.quat.x * Right.quat.x + Left.quat.y * Right.quat.y + Left.quat.z * Right.quat.z + Left.quat.w * Right.quat.w;
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

    return Quaternion(Vector4(Scale0 * Left.quat.x + Scale1 * Right.quat.x,
                              Scale0 * Left.quat.y + Scale1 * Right.quat.y,
                              Scale0 * Left.quat.z + Scale1 * Right.quat.z,
                              Scale0 * Left.quat.w + Scale1 * Right.quat.w));
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

Vector4 Quaternion::ToAxisAngle(Quaternion _quat) {
    
    Quaternion q = _quat;
    if (abs(_quat.quat.w) > 1.0f)
        q = Normalize(q);

    Vector4 Result = Vector4(0.0f, 0.0f, 0.0f, 2.0f * acos(q.quat.w));
    
    float den = sqrt(1.0f - (q.quat.w * q.quat.w));

    if (den > 0.0001f)
        Result.Xyz(q.quat.Xyz() / den);
    else
        Result.Xyz(Vector3(1.0f, 0.0f, 0.0f));

    return Result;
}

Matrix4 Quaternion::ToMatrix4(Quaternion quat) {
    Matrix3 mat;
    float qxx = quat.quat.x * quat.quat.x;
    float qyy = quat.quat.y * quat.quat.y;
    float qzz = quat.quat.z * quat.quat.z;

    float qxz = quat.quat.x * quat.quat.z;
    float qxy = quat.quat.x * quat.quat.y;
    float qyz = quat.quat.y * quat.quat.z;

    float qwx = quat.quat.w * quat.quat.x;
    float qwy = quat.quat.w * quat.quat.y;
    float qwz = quat.quat.w * quat.quat.z;

    mat.col0 = { 1.0f - 2.0f * (qyy + qzz), 2.0f * (qxy + qwz), 2.0f * (qxz - qwy) };
    mat.col1 = { 2.0f * (qxy - qwz), 1.0f - 2.0f * (qxx + qzz), 2.0f * (qyz + qwx) };
    mat.col2 = { 2.0f * (qxz + qwy), 2.0f * (qyz - qwx), 1.0f - 2.0f * (qxx + qyy) };

    return Matrix4(mat);
}

Quaternion Quaternion::operator*(float right) {

    return Quaternion(Vector4(
            quat.x * right,
            quat.y * right,
            quat.z * right,
            quat.w * right
    )); 
}

Quaternion Quaternion::operator*(Quaternion right) {

    return Quaternion(Vector4(
            quat.w * right.quat.x + quat.x * right.quat.w + quat.y * right.quat.z - quat.z * right.quat.y,
            quat.w * right.quat.y + quat.y * right.quat.w + quat.z * right.quat.x - quat.x * right.quat.z,
            quat.w * right.quat.z + quat.z * right.quat.w + quat.x * right.quat.y - quat.y * right.quat.x,
            quat.w * right.quat.w - quat.x * right.quat.x - quat.y * right.quat.y - quat.z * right.quat.z
    ));
}

Quaternion Quaternion::operator+(Quaternion right) {

    return Quaternion(Vector4(
            quat.x + right.quat.x,
            quat.y + right.quat.y,
            quat.z + right.quat.z,
            quat.w + right.quat.w));
}

Vector3 Quaternion::operator*(Vector3 right) {

    Vector3 Xyz = quat.Xyz();
    Vector3 Uv = Cross(Xyz, right);
    Vector3 Uuv = Cross(Xyz, Uv);

    return right + ((Uv * quat.w) + Uuv) * 2.0f;
}