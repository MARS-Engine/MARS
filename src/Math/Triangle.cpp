#include "Triangle.hpp"

Triangle::Triangle(Vector3 pos1, Vector3 pos2, Vector3 pos3) {
    Pos1 = pos1;
    Pos2 = pos2;
    Pos3 = pos3;
}

Triangle Triangle::operator+(const Vector3& other) const {
    return Triangle(Pos1 + other, Pos2 + other, Pos3 + other);
}

Triangle Triangle::operator*(const Matrix4& other) const {
    return Triangle(other * Pos1, other * Pos2, other  * Pos3);
}