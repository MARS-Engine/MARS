#include "vector4.hpp"

vector4::vector4() {

    x = y = z = w = 0;
}

vector4::vector4(vector2 xy, float _z, float _w) {

    x = xy.x;
    y = xy.y;
    z = _z;
    w = _w;
}

vector4::vector4(vector2 xy, vector2 zw) {
    x = xy.x;
    y = xy.y;
    z = zw.x;
    w = zw.y;
}

vector4::vector4(vector3 xyz) {

    x = xyz.x;
    y = xyz.y;
    z = xyz.z;
    w = 1;
}

vector4::vector4(vector3 xyz, float _w) {

    x = xyz.x;
    y = xyz.y;
    z = xyz.z;
    w = _w;
}

vector4::vector4(float value) {

    x = y = z = w = value;
}

vector4::vector4(float _x, float _y, float _z) {

    x = _x;
    y = _y;
    z = _z;
    w = 1;
}

vector4::vector4(float _x, float _y, float _z, float _w) {

    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

void vector4::xyz(vector3 xyz) {
    x = xyz.x;
    y = xyz.y;
    z = xyz.z;
}

vector3 vector4::xyz() const {

    return {x, y, z};
}

vector2 vector4::xy() const {

    return {x, y};
}

vector4 vector4::operator+(const vector4& right) const {

    return {x + right.x, y + right.y, z + right.z, w + right.w};
}

vector4 vector4::operator-(const vector4& right) const {

    return {x - right.x, y - right.y, z - right.z, w - right.w};
}

vector4 vector4::operator*(const float& right) const {

    return {x * right, y * right, z * right, w * right};
}

vector4 vector4::operator*(const vector4& right) const {

    return {x * right.x, y * right.y, z * right.z, w * right.w};
}

vector4& vector4::operator+=(const vector4& right) {

    x += right.x;
    y += right.y;
    z += right.z;
    w += right.w;
    return *this;
}

vector4& vector4::operator*=(const float& right) {

    x *= right;
    y *= right;
    z *= right;
    w *= right;
    return *this;
}

float& vector4::operator[](int index) {

    switch (index) {
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