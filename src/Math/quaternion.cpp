#include "quaternion.hpp"
#include "math.hpp"
#include "matrix3.hpp"
#include <math.h>

quaternion::quaternion() {
    value = vector4(0, 0, 0, 1);
}

quaternion::quaternion(vector4 _value) {
    value = _value;
}

quaternion::quaternion(vector3 xyz, float W) {
    value = vector4(xyz, W);
}


float quaternion::length() {
    return sqrt(value.w * value.w + value.xyz().length_squared());
}

vector3 quaternion::to_euler() {
    return {pitch(*this), yaw(*this), roll(*this)}; //Note sure if this works, euler should be avoided regardless
}

float quaternion::pitch(quaternion quat) {
    float y = 2.0f * (quat.value.y * quat.value.z + quat.value.w * quat.value.x);
    float x = quat.value.w * quat.value.w - quat.value.x * quat.value.x - quat.value.y * quat.value.y + quat.value.z * quat.value.z;
    if (equals(x, 0.0f, std::numeric_limits<float>::epsilon()) && equals(y, 0.0f, std::numeric_limits<float>::epsilon()))
        return 2.0f * atan2f(quat.value.x, quat.value.w);
    return atan2f(y, x);
}

float quaternion::yaw(quaternion quat) {
    return asin(std::clamp(-2.0f * (quat.value.x * quat.value.z - quat.value.w * quat.value.y), -1.0f, 1.0f));
}

float quaternion::roll(quaternion quat) {
    atan2(2.0f * (quat.value.x * quat.value.y + quat.value.w * quat.value.z), quat.value.w * quat.value.w + quat.value.x * quat.value.x - quat.value.y * quat.value.y - quat.value.z * quat.value.z);
}

quaternion quaternion::euler_to_quaternion(vector3 euler) {

    vector3 c = vector3(cosf(euler.x * deg2rad * 0.5f), cosf(euler.y * deg2rad * 0.5f), cosf(euler.z * deg2rad * 0.5f));
    vector3 s = vector3(sinf(euler.x * deg2rad * 0.5f), sinf(euler.y * deg2rad * 0.5f), sinf(euler.z * deg2rad * 0.5f));

    vector4 q;
    q.w = c.x * c.y * c.z + s.x * s.y * s.z;
    q.x = s.x * c.y * c.z - c.x * s.y * s.z;
    q.y = c.x * s.y * c.z + s.x * c.y * s.z;
    q.z = c.x * c.y * s.z - s.x * s.y * c.z;

    return quaternion(q);
}

quaternion CreateFromAxisAngle(vector3 axis, float angle) {
    float halfAngle = angle * .5f;
    float s = sin(halfAngle);
    quaternion q;
    q.value.x = axis.x * s;
    q.value.y = axis.y * s;
    q.value.z = axis.z * s;
    q.value.w = cos(halfAngle);
    return q;
}

quaternion quaternion::look_rotation(vector3 target, vector3 up_direction) {

    vector3 forwardVector = normalize(up_direction - target);

    float dot = vector3::dot(vector3(0, 0, 1), forwardVector);

    if (abs(dot - (-1.0f)) < 0.000001f)
        return quaternion(vector4(0.0f , 1.0f, 0.0f, M_PI));
    if (abs(dot - (1.0f)) < 0.000001f)
        return quaternion();

    float rotAngle = acos(dot);
    vector3 rotAxis = cross(vector3(0, 0, 1), forwardVector);
    rotAxis = normalize(rotAxis);
    return CreateFromAxisAngle(rotAxis, rotAngle);
}

quaternion quaternion::slerp_not_normalized(quaternion left, quaternion right, float slerp) {

    float RawCosom = left.value.x * right.value.x + left.value.y * right.value.y + left.value.z * right.value.z + left.value.w * right.value.w;
    float Cosom = RawCosom >= 0.f ? RawCosom : -RawCosom;

    float Scale0, Scale1;

    if (Cosom < 0.9999f) {

        float Omega = acos(Cosom);
        float InvSin = 1.f/sin(Omega);
        Scale0 = sin((1.f - slerp) * Omega) * InvSin;
        Scale1 = sin(slerp * Omega) * InvSin;
    }
    else {
        Scale0 = 1.0f - slerp;
        Scale1 = slerp;
    }

    Scale1 = RawCosom >= 0.f ? Scale1 : -Scale1;

    return quaternion(vector4(Scale0 * left.value.x + Scale1 * right.value.x,
                              Scale0 * left.value.y + Scale1 * right.value.y,
                              Scale0 * left.value.z + Scale1 * right.value.z,
                              Scale0 * left.value.w + Scale1 * right.value.w));
}

quaternion quaternion::slerp(quaternion left, quaternion right, float slerp) {
    return normalize(slerp_not_normalized(left, right, slerp));
}

quaternion quaternion::lerp(quaternion left, quaternion right, float time) {

    return left * (1.0f - time) + (right * time);
}

quaternion quaternion::from_axis_angle(vector3 axis, float angle) {
    if (axis.length_squared() == 0.0f)
        return quaternion();

    angle *= 0.5f;
    return normalize(quaternion(normalize(axis) * sin(angle), cos(angle)));
}

vector4 quaternion::to_axis_angle(quaternion quat) {
    
    quaternion q = quat;
    if (abs(quat.value.w) > 1.0f)
        q = normalize(q);

    vector4 Result = vector4(0.0f, 0.0f, 0.0f, 2.0f * acos(q.value.w));
    
    float den = sqrt(1.0f - (q.value.w * q.value.w));

    if (den > 0.0001f)
        Result.xyz(q.value.xyz() / den);
    else
        Result.xyz(vector3(1.0f, 0.0f, 0.0f));

    return Result;
}

matrix4 quaternion::to_matrix4(quaternion quat) {
    matrix3 mat;
    float qxx = quat.value.x * quat.value.x;
    float qyy = quat.value.y * quat.value.y;
    float qzz = quat.value.z * quat.value.z;

    float qxz = quat.value.x * quat.value.z;
    float qxy = quat.value.x * quat.value.y;
    float qyz = quat.value.y * quat.value.z;

    float qwx = quat.value.w * quat.value.x;
    float qwy = quat.value.w * quat.value.y;
    float qwz = quat.value.w * quat.value.z;

    mat.col0 = { 1.0f - 2.0f * (qyy + qzz), 2.0f * (qxy + qwz), 2.0f * (qxz - qwy) };
    mat.col1 = { 2.0f * (qxy - qwz), 1.0f - 2.0f * (qxx + qzz), 2.0f * (qyz + qwx) };
    mat.col2 = { 2.0f * (qxz + qwy), 2.0f * (qyz - qwx), 1.0f - 2.0f * (qxx + qyy) };

    return matrix4(mat);
}

quaternion quaternion::operator*(float right) {

    return quaternion(vector4(
            value.x * right,
            value.y * right,
            value.z * right,
            value.w * right
    )); 
}

quaternion quaternion::operator*(quaternion right) {

    return quaternion(vector4(
            value.w * right.value.x + value.x * right.value.w + value.y * right.value.z - value.z * right.value.y,
            value.w * right.value.y + value.y * right.value.w + value.z * right.value.x - value.x * right.value.z,
            value.w * right.value.z + value.z * right.value.w + value.x * right.value.y - value.y * right.value.x,
            value.w * right.value.w - value.x * right.value.x - value.y * right.value.y - value.z * right.value.z
    ));
}

quaternion quaternion::operator+(quaternion right) {

    return quaternion(vector4(
            value.x + right.value.x,
            value.y + right.value.y,
            value.z + right.value.z,
            value.w + right.value.w));
}

vector3 quaternion::operator*(vector3 right) {

    vector3 Xyz = value.xyz();
    vector3 Uv = cross(Xyz, right);
    vector3 Uuv = cross(Xyz, Uv);

    return right + ((Uv * value.w) + Uuv) * 2.0f;
}

bool quaternion::operator==(const quaternion& other) const {

    return value.x == other.value.x && value.y == other.value.y && value.z == other.value.z && value.w == other.value.w;
}


bool quaternion::operator!=(const quaternion& other) const {

    return !(value.x == other.value.x && value.y == other.value.y && value.z == other.value.z && value.w == other.value.w);
}
