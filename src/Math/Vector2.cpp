#include "Vector2.hpp"
#include <math.h>

Vector2::Vector2() {

    x = y = 0.0f;
}
   
Vector2::Vector2(float _x, float _y) {

    x = _x;
    y = _y;
}

float Vector2::AngleBetween(Vector2 Right, Vector2 Left) {
    float s = Right.x * Left.y - Left.x * Right.y;
    float c = Right.x * Left.x + Left.y * Right.y;

    return atan2f(s, c) * (180 / M_PI);
}

bool Vector2::operator==(const Vector2& other) const {

    return x == other.x && y == other.y;
}

bool Vector2::operator!=(const Vector2& other) const {

    return !(x == other.x && y == other.y);
}

Vector2 Vector2::operator+(const Vector2& right) const {

    return {x + right.x, y + right.y};
}

Vector2 Vector2::operator+(const float& right) const {

    return {x + right, y + right};
}

Vector2 Vector2::operator-(const Vector2& right) const {

    return {x - right.x, y - right.y};
}

Vector2 Vector2::operator-(const float& right) const {

    return {x - right, y - right};
}

Vector2 Vector2::operator/(const Vector2& right) const {

    return {x / right.x, y / right.y};
}

Vector2 Vector2::operator*(const Vector2& right) const {

    return {x * right.x, y * right.y};
}

Vector2 Vector2::operator*(const float& right) const {

    return {x * right, y * right};
}

Vector2 Vector2::operator/(const float& right) const {

    return {x / right, y / right};
}

bool Vector2::operator>(const Vector2& right) const {
    return x > right.x && y > right.y;
}

bool Vector2::operator<(const Vector2& right) const {
    return x < right.x && y < right.y;
}