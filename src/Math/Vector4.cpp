#include "Vector4.hpp"

Vector4::Vector4() {

    x = y = z = w = 0;
}

Vector4::Vector4(Vector2 Xy, float _z, float _w) {

    x = Xy.x;
    y = Xy.y;
    z = _z;
    w = _w;
}

Vector4::Vector4(Vector2 Xy, Vector2 Zw) {
    x = Xy.x;
    y = Xy.y;
    z = Zw.x;
    w = Zw.y;
}

Vector4::Vector4(Vector3 Xyz) {

    x = Xyz.x;
    y = Xyz.y;
    z = Xyz.z;
    w = 1;
}

Vector4::Vector4(Vector3 Xyz, float _w) {

    x = Xyz.x;
    y = Xyz.y;
    z = Xyz.z;
    w = _w;
}

Vector4::Vector4(float value) {

    x = y = z = w = value;
}

Vector4::Vector4(float _x, float _y, float _z) {

    x = _x;
    y = _y;
    z = _z;
    w = 1;
}

Vector4::Vector4(float _x, float _y, float _z, float _w) {

    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

void Vector4::Xyz(Vector3 Xyz) {
    x = Xyz.x;
    y = Xyz.y;
    z = Xyz.z;
}

Vector3 Vector4::Xyz() const {

    return {x, y, z};
}

Vector2 Vector4::Xy() const {

    return {x, y};
}

Vector4 Vector4::operator+(const Vector4& right) const {

    return {x + right.x, y + right.y, z + right.z, w + right.w};
}

Vector4 Vector4::operator-(const Vector4& right) const {

    return {x - right.x, y - right.y, z - right.z, w - right.w};
}

Vector4 Vector4::operator*(const float& right) const {

    return {x * right, y * right, z * right, w * right};
}

Vector4 Vector4::operator*(const Vector4& right) const {

    return {x * right.x, y * right.y, z * right.z, w * right.w};
}

Vector4& Vector4::operator+=(const Vector4& right) {

    x += right.x;
    y += right.y;
    z += right.z;
    w += right.w;
    return *this;
}

Vector4& Vector4::operator*=(const float& right) {

    x *= right;
    y *= right;
    z *= right;
    w *= right;
    return *this;
}

float& Vector4::operator[](int Index) {

    switch (Index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
        default:
            return x;
    }
}