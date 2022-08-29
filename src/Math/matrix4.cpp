#include "matrix4.hpp"
#include "matrix3.hpp"
#include "math.h"

matrix4::matrix4() {
    
    col0 = vector4(1, 0, 0, 0);
    col1 = vector4(0, 1, 0, 0);
    col2 = vector4(0, 0, 1, 0);
    col3 = vector4(0, 0, 0, 1);
}

matrix4::matrix4(float value) {
    
    col0 = vector4(value, 0, 0, 0);
    col1 = vector4(0, value, 0, 0);
    col2 = vector4(0, 0, value, 0);
    col3 = vector4(0, 0, 0, value);
}

matrix4::matrix4(matrix3 value) {

    col0 = vector4(value.col0 , 0);
    col1 = vector4(value.col1 , 0);
    col2 = vector4(value.col2 , 0);
    col3 = vector4(0, 0, 0, 1);
}

matrix4::matrix4(vector4 _col0, vector4 _col1, vector4 _col2, vector4 _col3) {

    col0 = _col0;
    col1 = _col1;
    col2 = _col2;
    col3 = _col3;
}

matrix4 matrix4::inverse(matrix4 value) {

    float Coef00 = value.col2.z * value.col3.w - value.col3.z * value.col2.w;
    float Coef02 = value.col1.z * value.col3.w - value.col3.z * value.col1.w;
    float Coef03 = value.col1.z * value.col2.w - value.col2.z * value.col1.w;

    float Coef04 = value.col2.y * value.col3.w - value.col3.y * value.col2.w;
    float Coef06 = value.col1.y * value.col3.w - value.col3.y * value.col1.w;
    float Coef07 = value.col1.y * value.col2.w - value.col2.y * value.col1.w;

    float Coef08 = value.col2.y * value.col3.z - value.col3.y * value.col2.z;
    float Coef10 = value.col1.y * value.col3.z - value.col3.y * value.col1.z;
    float Coef11 = value.col1.y * value.col2.z - value.col2.y * value.col1.z;

    float Coef12 = value.col2.x * value.col3.w - value.col3.x * value.col2.w;
    float Coef14 = value.col1.x * value.col3.w - value.col3.x * value.col1.w;
    float Coef15 = value.col1.x * value.col2.w - value.col2.x * value.col1.w;

    float Coef16 = value.col2.x * value.col3.z - value.col3.x * value.col2.z;
    float Coef18 = value.col1.x * value.col3.z - value.col3.x * value.col1.z;
    float Coef19 = value.col1.x * value.col2.z - value.col2.x * value.col1.z;

    float Coef20 = value.col2.x * value.col3.y - value.col3.x * value.col2.y;
    float Coef22 = value.col1.x * value.col3.y - value.col3.x * value.col1.y;
    float Coef23 = value.col1.x * value.col2.y - value.col2.x * value.col1.y;

    vector4 Fac0(Coef00, Coef00, Coef02, Coef03);
    vector4 Fac1(Coef04, Coef04, Coef06, Coef07);
    vector4 Fac2(Coef08, Coef08, Coef10, Coef11);
    vector4 Fac3(Coef12, Coef12, Coef14, Coef15);
    vector4 Fac4(Coef16, Coef16, Coef18, Coef19);
    vector4 Fac5(Coef20, Coef20, Coef22, Coef23);

    vector4 Vec0(value.col1.x, value.col0.x, value.col0.x, value.col0.x);
    vector4 Vec1(value.col1.y, value.col0.y, value.col0.y, value.col0.y);
    vector4 Vec2(value.col1.z, value.col0.z, value.col0.z, value.col0.z);
    vector4 Vec3(value.col1.w, value.col0.w, value.col0.w, value.col0.w);

    vector4 Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
    vector4 Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
    vector4 Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
    vector4 Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

    vector4 SignA(+1, -1, +1, -1);
    vector4 SignB(-1, +1, -1, +1);
    matrix4 Inverse(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

    vector4 Row0(Inverse.col0.x, Inverse.col1.x, Inverse.col2.x, Inverse.col3.x);

    vector4 Dot0(value.col0 * Row0);
    
    float Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);
    float OOD = 1.0f / Dot1;
    return Inverse * OOD;
}

matrix4 matrix4::inverse_transpose(matrix4 value) {
    float SubFactor00 = value[2][2] * value[3][3] - value[3][2] * value[2][3];
    float SubFactor01 = value[2][1] * value[3][3] - value[3][1] * value[2][3];
    float SubFactor02 = value[2][1] * value[3][2] - value[3][1] * value[2][2];

    float SubFactor03 = value[2][0] * value[3][3] - value[3][0] * value[2][3];
    float SubFactor04 = value[2][0] * value[3][2] - value[3][0] * value[2][2];
    float SubFactor05 = value[2][0] * value[3][1] - value[3][0] * value[2][1];

    float SubFactor06 = value[1][2] * value[3][3] - value[3][2] * value[1][3];
    float SubFactor07 = value[1][1] * value[3][3] - value[3][1] * value[1][3];
    float SubFactor08 = value[1][1] * value[3][2] - value[3][1] * value[1][2];

    float SubFactor09 = value[1][0] * value[3][3] - value[3][0] * value[1][3];
    float SubFactor10 = value[1][0] * value[3][2] - value[3][0] * value[1][2];
    float SubFactor11 = value[1][0] * value[3][1] - value[3][0] * value[1][1];

    float SubFactor12 = value[1][2] * value[2][3] - value[2][2] * value[1][3];
    float SubFactor13 = value[1][1] * value[2][3] - value[2][1] * value[1][3];
    float SubFactor14 = value[1][1] * value[2][2] - value[2][1] * value[1][2];

    float SubFactor15 = value[1][0] * value[2][3] - value[2][0] * value[1][3];
    float SubFactor16 = value[1][0] * value[2][2] - value[2][0] * value[1][2];
    float SubFactor17 = value[1][0] * value[2][1] - value[2][0] * value[1][1];

    matrix4 Inverse;
    Inverse[0][0] = + (value[1][1] * SubFactor00 - value[1][2] * SubFactor01 + value[1][3] * SubFactor02);
    Inverse[0][1] = - (value[1][0] * SubFactor00 - value[1][2] * SubFactor03 + value[1][3] * SubFactor04);
    Inverse[0][2] = + (value[1][0] * SubFactor01 - value[1][1] * SubFactor03 + value[1][3] * SubFactor05);
    Inverse[0][3] = - (value[1][0] * SubFactor02 - value[1][1] * SubFactor04 + value[1][2] * SubFactor05);

    Inverse[1][0] = - (value[0][1] * SubFactor00 - value[0][2] * SubFactor01 + value[0][3] * SubFactor02);
    Inverse[1][1] = + (value[0][0] * SubFactor00 - value[0][2] * SubFactor03 + value[0][3] * SubFactor04);
    Inverse[1][2] = - (value[0][0] * SubFactor01 - value[0][1] * SubFactor03 + value[0][3] * SubFactor05);
    Inverse[1][3] = + (value[0][0] * SubFactor02 - value[0][1] * SubFactor04 + value[0][2] * SubFactor05);

    Inverse[2][0] = + (value[0][1] * SubFactor06 - value[0][2] * SubFactor07 + value[0][3] * SubFactor08);
    Inverse[2][1] = - (value[0][0] * SubFactor06 - value[0][2] * SubFactor09 + value[0][3] * SubFactor10);
    Inverse[2][2] = + (value[0][0] * SubFactor07 - value[0][1] * SubFactor09 + value[0][3] * SubFactor11);
    Inverse[2][3] = - (value[0][0] * SubFactor08 - value[0][1] * SubFactor10 + value[0][2] * SubFactor11);

    Inverse[3][0] = - (value[0][1] * SubFactor12 - value[0][2] * SubFactor13 + value[0][3] * SubFactor14);
    Inverse[3][1] = + (value[0][0] * SubFactor12 - value[0][2] * SubFactor15 + value[0][3] * SubFactor16);
    Inverse[3][2] = - (value[0][0] * SubFactor13 - value[0][1] * SubFactor15 + value[0][3] * SubFactor17);
    Inverse[3][3] = + (value[0][0] * SubFactor14 - value[0][1] * SubFactor16 + value[0][2] * SubFactor17);

    matrix4 Determinant =
            + value[0][0] * Inverse[0][0]
            + value[0][1] * Inverse[0][1]
            + value[0][2] * Inverse[0][2]
            + value[0][3] * Inverse[0][3];

    Inverse /= Determinant;

    return Inverse;
}

matrix4 matrix4::translate(vector3 translation) {

    return translate(matrix4(1), translation);
}

matrix4 matrix4::translate(matrix4 target, vector3 translation) {

    matrix4 Result(target);

    Result.col3 = target.col0 * translation.x + target.col1 * translation.y + target.col2 * translation.z + target.col3;
    return Result;
}

matrix4 matrix4::scale(vector3 _scale) {

    return scale(matrix4(1), _scale);
}

matrix4 matrix4::scale(matrix4 _target, vector3 _scale) {
    
    matrix4 Result(_target);
    
    Result.col0 *= _scale.x;
    Result.col1 *= _scale.y;
    Result.col2 *= _scale.z;
    
    return Result;
}

matrix4 matrix4::look_at_lh(vector3 eye, vector3 center, vector3 up)  {

    vector3 f = normalize(center - eye);
    vector3 s = normalize(cross(up, f));
    vector3 u = cross(f, s);

    matrix4 Result = matrix4(1);
    Result.col0.x = s.x;
    Result.col1.x = s.y;
    Result.col2.x = s.z;
    Result.col0.y = u.x;
    Result.col1.y = u.y;
    Result.col2.y = u.z;
    Result.col0.z = f.x;
    Result.col1.z = f.y;
    Result.col2.z = f.z;
    Result.col3.x = -vector3::dot(s, eye);
    Result.col3.y = -vector3::dot(u, eye);
    Result.col3.z = -vector3::dot(f, eye);

    return Result;
}

matrix4 matrix4::look_at(vector3 eye, vector3 center, vector3 up)  {

    vector3 f = normalize(center - eye);
    vector3 s = normalize(cross(f, up));
    vector3 u = cross(s, f);

    matrix4 Result = matrix4(1);
    Result.col0.x = s.x;
    Result.col1.x = s.y;
    Result.col2.x = s.z;
    Result.col0.y = u.x;
    Result.col1.y = u.y;
    Result.col2.y = u.z;
    Result.col0.z = -f.x;
    Result.col1.z = -f.y;
    Result.col2.z = -f.z;
    Result.col3.x = -vector3::dot(s, eye);
    Result.col3.y = -vector3::dot(u, eye);
    Result.col3.z = vector3::dot(f, eye);

    return Result;
}

matrix4 matrix4::perspective_fov_lh(float fov, float width, float height, float z_near, float z_far) {

    if (width < 0) {
        debug::alert("PespectiveFovLH Faild Because Width Is Negative");
        return matrix4(1);
    }
    else if(height < 0) {
        debug::alert("PespectiveFovLH Faild Because Height Is Negative");
        return matrix4(1);
    }
    else if(fov < 0) {
        debug::alert("PespectiveFovLH Faild Because Fov Is Negative");
        return matrix4(1);
    }

    float rad = fov;
    float h = cosf(0.5f * rad) / sinf(0.5f * rad);
    float w = h * height / width;

    matrix4 Result(0);
    Result.col0.x = w;
    Result.col1.y = h;
    Result.col2.z = (z_far + z_near) / (z_far - z_near);
    Result.col2.w = 1.0f;
    Result.col3.z = - (2.0f * z_far * z_near) / (z_far - z_near);
    return Result;
}

matrix4 matrix4::perspective_lh(float fov, float aspect, float z_near, float z_far) {

    if (abs(aspect - numeric_limits<float>::epsilon()) < 0.0f) {
        debug::alert("PespectiveFov Faild Because Width Is Negative");
        return matrix4(1);
    }

    float tanHalfFovy = tan(fov / 2.0f);

    matrix4 Result(0);
    Result[0][0] = 1.0f / (aspect * tanHalfFovy);
    Result[1][1] = 1.0f / (tanHalfFovy);
    Result[2][2] = z_far / (z_far - z_near);
    Result[2][3] = 1.0f;
    Result[3][2] = - (z_far * z_near) / (z_far - z_near);
    return Result;
}

matrix4 matrix4::ortho(float left, float right, float bottom, float top, float z_near, float z_far) {

    auto Result = matrix4(1);
    Result[0][0] = 2.0f / (right - left);
    Result[1][1] = 2.0f / (top - bottom);
    Result[2][2] = - 2.0f / (z_far - z_near);
    Result[3][0] = - (right + left) / (right - left);
    Result[3][1] = - (top + bottom) / (top - bottom);
    Result[3][2] = - (z_far + z_near) / (z_far - z_near);
    return Result;
}

matrix4 matrix4::ortho_lh(float left, float right, float bottom, float top, float z_near, float z_far) {

    auto Result = matrix4(1);
    Result[0][0] = 2.0f / (right - left);
    Result[1][1] = 2.0f / (top - bottom);
    Result[2][2] = 1.0f / (z_far - z_near);
    Result[3][0] = - (right + left) / (right - left);
    Result[3][1] = - (top + bottom) / (top - bottom);
    Result[3][2] = - z_near / (z_far - z_near);
    return Result;
}

matrix4 matrix4::create_from_axis_angle(vector3 axis, float angle) {
    vector3 NormAxis = normalize(axis);
    float axisX = NormAxis.x;
    float axisY = NormAxis.y;
    float axisZ = NormAxis.z;

    float Cos = cos(-angle);
    float Sin = sin(-angle);
    float t = 1.0f - Cos;

    float tXX = t * axisX * axisX;
    float tXY = t * axisX * axisY;
    float tXZ = t * axisX * axisZ;
    float tYY = t * axisY * axisY;
    float tYZ = t * axisY * axisZ;
    float tZZ = t * axisZ * axisZ;
    
    float sinX = Sin * axisX;
    float sinY = Sin * axisY;
    float sinZ = Sin * axisZ;

    matrix4 Result = matrix4();
    Result.col0.x = tXX + Cos;
    Result.col0.y = tXY - sinZ;
    Result.col0.z = tXZ + sinY;
    Result.col0.w = 0;
    Result.col1.x = tXY + sinZ;
    Result.col1.y = tYY + Cos;
    Result.col1.z = tYZ - sinX;
    Result.col1.w = 0;
    Result.col2.x = tXZ - sinY;
    Result.col2.y = tYZ + sinX;
    Result.col2.z = tZZ + Cos;
    Result.col2.w = 0;
    Result.col3 = vector4(0, 0, 0, 1);

    return Result;
}

matrix4 matrix4::from_quaternion(quaternion quat) {
    vector4 AxisAngle = quaternion::to_axis_angle(quat);
    return create_from_axis_angle(AxisAngle.xyz(), AxisAngle.w);
}

matrix4 matrix4::operator*(const matrix4& right) const {

    vector4 SrcA0 = col0;
    vector4 SrcA1 = col1;
    vector4 SrcA2 = col2;
    vector4 SrcA3 = col3;

    vector4 SrcB0 = right.col0;
    vector4 SrcB1 = right.col1;
    vector4 SrcB2 = right.col2;
    vector4 SrcB3 = right.col3;

    matrix4 Result(0);
    Result.col0 = SrcA0 * SrcB0.x + SrcA1 * SrcB0.y + SrcA2 * SrcB0.z + SrcA3 * SrcB0.w;
    Result.col1 = SrcA0 * SrcB1.x + SrcA1 * SrcB1.y + SrcA2 * SrcB1.z + SrcA3 * SrcB1.w;
    Result.col2 = SrcA0 * SrcB2.x + SrcA1 * SrcB2.y + SrcA2 * SrcB2.z + SrcA3 * SrcB2.w;
    Result.col3 = SrcA0 * SrcB3.x + SrcA1 * SrcB3.y + SrcA2 * SrcB3.z + SrcA3 * SrcB3.w;
    return Result;
}

matrix4 matrix4::operator*(const float& right) const {

    return matrix4(
        col0 * right,
        col1 * right,
        col2 * right,
        col3 * right
    );
}

vector4 matrix4::operator*(const vector4& right) const {

    vector4 Mov0(right.x);
    vector4 Mov1(right.y);
    vector4 Mul0 = col0 * Mov0;
    vector4 Mul1 = col1 * Mov1;
    vector4 Add0 = Mul0 + Mul1;

    vector4 Mov2(right.z);
    vector4 Mov3(right.w);
    vector4 Mul2 = col2 * Mov2;
    vector4 Mul3 = col3 * Mov3;
    vector4 Add1 = Mul2 + Mul3;
    return Add0 + Add1;

}

vector3 matrix4::operator*(const vector3& right) const {
    return (*this * vector4(right)).xyz();
}

matrix4 matrix4::operator/=(const matrix4& right) {
    return  *this *= inverse(right);
}

matrix4 matrix4::operator*=(const matrix4& right) {
    return (*this = *this * right);
}

matrix4 matrix4::operator=(const matrix4& right) {
    this->col0 = right.col0;
    this->col1 = right.col1;
    this->col2 = right.col2;
    this->col3 = right.col3;
    return *this;
}


vector4& matrix4::operator[](int Index) {

    switch (Index) {
        case 0:
            return col0;
        case 1:
            return col1;
        case 2:
            return col2;
        case 3:
            return col3;
        default:
            return col0;
    }
}