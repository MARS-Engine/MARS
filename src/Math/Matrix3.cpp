#include "Matrix3.hpp"

Matrix3::Matrix3() {
    col0 = { 1.0f, 0.0f, 0.0f};
    col1 = { 0.0f, 1.0f, 0.0f};
    col2 = { 0.0f, 0.0f, 1.0f};
}

Matrix3::Matrix3(Matrix4 v) {
    col0 = v.col0.Xyz();
    col1 = v.col1.Xyz();
    col2 = v.col2.Xyz();
}