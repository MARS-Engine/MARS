#include "vector2.hpp"
#include <math.h>

vector2::vector2() {

    x = y = 0.0f;
}
   
vector2::vector2(float _x, float _y) {

    x = _x;
    y = _y;
}

float vector2::angle_between(vector2 Right, vector2 Left) {
    float s = Right.x * Left.y - Left.x * Right.y;
    float c = Right.x * Left.x + Left.y * Right.y;

    return atan2f(s, c) * (180 / M_PI);
}

bool vector2::operator==(const vector2& other) const {

    return x == other.x && y == other.y;
}

bool vector2::operator!=(const vector2& other) const {

    return !(x == other.x && y == other.y);
}

vector2 vector2::operator+(const vector2& right) const {

    return {x + right.x, y + right.y};
}

vector2 vector2::operator+(const float& right) const {

    return {x + right, y + right};
}

vector2 vector2::operator-(const vector2& right) const {

    return {x - right.x, y - right.y};
}

vector2 vector2::operator-(const float& right) const {

    return {x - right, y - right};
}

vector2 vector2::operator/(const vector2& right) const {

    return {x / right.x, y / right.y};
}

vector2 vector2::operator*(const vector2& right) const {

    return {x * right.x, y * right.y};
}

vector2 vector2::operator*(const float& right) const {

    return {x * right, y * right};
}

vector2 vector2::operator/(const float& right) const {

    return {x / right, y / right};
}

bool vector2::operator>(const vector2& right) const {
    return x > right.x && y > right.y;
}

bool vector2::operator<(const vector2& right) const {
    return x < right.x && y < right.y;
}

size_t vector2::operator()(const vector2 &v) const {
    return ((hash<float>()(v.x) ^ (hash<float>()(v.y) << 1)) >> 1);
}