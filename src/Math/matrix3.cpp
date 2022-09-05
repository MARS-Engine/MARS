#include "matrix3.hpp"

matrix3::matrix3() {
    col0 = { 1.0f, 0.0f, 0.0f};
    col1 = { 0.0f, 1.0f, 0.0f};
    col2 = { 0.0f, 0.0f, 1.0f};
}

matrix3::matrix3(matrix4 v) {
    col0 = v.col0.xyz();
    col1 = v.col1.xyz();
    col2 = v.col2.xyz();
}