#include "Matrix3.hpp"

Matrix3::Matrix3(Matrix4 v) {
    col0 = v.col0.Xyz();
    col1 = v.col1.Xyz();
    col2 = v.col2.Xyz();
}