#include "triangle.hpp"

triangle::triangle(vector3 _pos1, vector3 _pos2, vector3 _pos3) {
    pos1 = _pos1;
    pos2 = _pos2;
    pos3 = _pos3;
}

triangle triangle::operator+(const vector3& other) const {
    return triangle(pos1 + other, pos2 + other, pos3 + other);
}

triangle triangle::operator*(const matrix4& other) const {
    return triangle(other * pos1, other * pos2, other * pos3);
}