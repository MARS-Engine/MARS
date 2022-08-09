#include "Matrix4.hpp"
#include "Matrix3.hpp"
#include "math.h"

Matrix4::Matrix4() {
    
    col0 = Vector4(1, 0, 0, 0);
    col1 = Vector4(0, 1, 0, 0);
    col2 = Vector4(0, 0, 1, 0);
    col3 = Vector4(0, 0, 0, 1);
}

Matrix4::Matrix4(float V) {
    
    col0 = Vector4(V, 0, 0, 0);
    col1 = Vector4(0, V, 0, 0);
    col2 = Vector4(0, 0, V, 0);
    col3 = Vector4(0, 0, 0, V);
}

Matrix4::Matrix4(Matrix3 V) {

    col0 = Vector4(V.col0 , 0);
    col1 = Vector4(V.col1 , 0);
    col2 = Vector4(V.col2 , 0);
    col3 = Vector4(0, 0, 0, 1);
}

Matrix4::Matrix4(Vector4 c0, Vector4 c1, Vector4 c2, Vector4 c3) {

    col0 = c0;
    col1 = c1;
    col2 = c2;
    col3 = c3;
}

Matrix4 Matrix4::Inverse(Matrix4 V) {

    float Coef00 = V.col2.z * V.col3.w - V.col3.z * V.col2.w;
    float Coef02 = V.col1.z * V.col3.w - V.col3.z * V.col1.w;
    float Coef03 = V.col1.z * V.col2.w - V.col2.z * V.col1.w;

    float Coef04 = V.col2.y * V.col3.w - V.col3.y * V.col2.w;
    float Coef06 = V.col1.y * V.col3.w - V.col3.y * V.col1.w;
    float Coef07 = V.col1.y * V.col2.w - V.col2.y * V.col1.w;

    float Coef08 = V.col2.y * V.col3.z - V.col3.y * V.col2.z;
    float Coef10 = V.col1.y * V.col3.z - V.col3.y * V.col1.z;
    float Coef11 = V.col1.y * V.col2.z - V.col2.y * V.col1.z;

    float Coef12 = V.col2.x * V.col3.w - V.col3.x * V.col2.w;
    float Coef14 = V.col1.x * V.col3.w - V.col3.x * V.col1.w;
    float Coef15 = V.col1.x * V.col2.w - V.col2.x * V.col1.w;

    float Coef16 = V.col2.x * V.col3.z - V.col3.x * V.col2.z;
    float Coef18 = V.col1.x * V.col3.z - V.col3.x * V.col1.z;
    float Coef19 = V.col1.x * V.col2.z - V.col2.x * V.col1.z;

    float Coef20 = V.col2.x * V.col3.y - V.col3.x * V.col2.y;
    float Coef22 = V.col1.x * V.col3.y - V.col3.x * V.col1.y;
    float Coef23 = V.col1.x * V.col2.y - V.col2.x * V.col1.y;

    Vector4 Fac0(Coef00, Coef00, Coef02, Coef03);
    Vector4 Fac1(Coef04, Coef04, Coef06, Coef07);
    Vector4 Fac2(Coef08, Coef08, Coef10, Coef11);
    Vector4 Fac3(Coef12, Coef12, Coef14, Coef15);
    Vector4 Fac4(Coef16, Coef16, Coef18, Coef19);
    Vector4 Fac5(Coef20, Coef20, Coef22, Coef23);

    Vector4 Vec0(V.col1.x, V.col0.x, V.col0.x, V.col0.x);
    Vector4 Vec1(V.col1.y, V.col0.y, V.col0.y, V.col0.y);
    Vector4 Vec2(V.col1.z, V.col0.z, V.col0.z, V.col0.z);
    Vector4 Vec3(V.col1.w, V.col0.w, V.col0.w, V.col0.w);

    Vector4 Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
    Vector4 Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
    Vector4 Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
    Vector4 Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

    Vector4 SignA(+1, -1, +1, -1);
    Vector4 SignB(-1, +1, -1, +1);
    Matrix4 Inverse(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

    Vector4 Row0(Inverse.col0.x, Inverse.col1.x, Inverse.col2.x, Inverse.col3.x);

    Vector4 Dot0(V.col0 * Row0);
    
    float Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);
    float OOD = 1.0f / Dot1;
    return Inverse * OOD;
}

Matrix4 Matrix4::Translate(Vector3 translation) {

    return Translate(Matrix4(1), translation);
}

Matrix4 Matrix4::Translate(Matrix4 Target, Vector3 translation) {

    Matrix4 Result(Target);

    Result.col3 = Target.col0 * translation.x + Target.col1 * translation.y + Target.col2 * translation.z + Target.col3;
    return Result;
}

Matrix4 Matrix4::Scale(Vector3 translation) {

    return Scale(Matrix4(1), translation);
}

Matrix4 Matrix4::Scale(Matrix4 Target, Vector3 scale) {
    
    Matrix4 Result(Target);
    
    Result.col0 *= scale.x;
    Result.col1 *= scale.y;
    Result.col2 *= scale.z;
    
    return Result;
}

Matrix4 Matrix4::LookAtLH(Vector3 Eye, Vector3 Center, Vector3 Up)  {

    Vector3 f = Normalize(Center - Eye);
    Vector3 s = Normalize(Cross(Up, f));
    Vector3 u = Cross(f, s);

    Matrix4 Result = Matrix4(1);
    Result.col0.x = s.x;
    Result.col1.x = s.y;
    Result.col2.x = s.z;
    Result.col0.y = u.x;
    Result.col1.y = u.y;
    Result.col2.y = u.z;
    Result.col0.z = f.x;
    Result.col1.z = f.y;
    Result.col2.z = f.z;
    Result.col3.x = -Dot(s, Eye);
    Result.col3.y = -Dot(u, Eye);
    Result.col3.z = -Dot(f, Eye);

    return Result;
}

Matrix4 Matrix4::LookAt(Vector3 Eye, Vector3 Center, Vector3 Up)  {

    Vector3 f = Normalize(Center - Eye);
    Vector3 s = Normalize(Cross(f, Up));
    Vector3 u = Cross(s, f);

    Matrix4 Result = Matrix4(1);
    Result.col0.x = s.x;
    Result.col1.x = s.y;
    Result.col2.x = s.z;
    Result.col0.y = u.x;
    Result.col1.y = u.y;
    Result.col2.y = u.z;
    Result.col0.z = -f.x;
    Result.col1.z = -f.y;
    Result.col2.z = -f.z;
    Result.col3.x = -Dot(s, Eye);
    Result.col3.y = -Dot(u, Eye);
    Result.col3.z = Dot(f, Eye);

    return Result;
}

Matrix4 Matrix4::PerspectiveFovLH(float Fov, float Width, float Height, float zNear, float zFar) {

    if (Width < 0) {
        Debug::Alert("PespectiveFovLH Faild Because Width Is Negative");
        return Matrix4(1);
    }
    else if(Height < 0) {
        Debug::Alert("PespectiveFovLH Faild Because Height Is Negative");
        return Matrix4(1);
    }
    else if(Fov < 0) {
        Debug::Alert("PespectiveFovLH Faild Because Fov Is Negative");
        return Matrix4(1);
    }

    float rad = Fov;
    float h = cosf(0.5f * rad) / sinf(0.5f * rad);
    float w = h * Height / Width;

    Matrix4 Result(0);
    Result.col0.x = w;
    Result.col1.y = h;
    Result.col2.z = (zFar + zNear) / (zFar - zNear);
    Result.col2.w = 1.0f;
    Result.col3.z = - (2.0f * zFar * zNear) / (zFar - zNear);
    return Result;
}

Matrix4 Matrix4::PerspectiveFov(float Fov, float Aspect, float zNear, float zFar) {

    if (abs(Aspect - numeric_limits<float>::epsilon()) < 0.0f) {
        Debug::Alert("PespectiveFov Faild Because Width Is Negative");
        return Matrix4(1);
    }

    float tanHalfFovy = tan(Fov / 2.0f);

    Matrix4 Result(0);
    Result.col0.x = 1.0f / (Aspect * tanHalfFovy);
    Result.col1.y = 1.0f / (tanHalfFovy);
    Result.col2.z = - (zFar + zNear) / (zFar - zNear);
    Result.col2.w = - 1.0f;
    Result.col3.z = - (2.0f * zFar * zNear) / (zFar - zNear);
    return Result;
}

Matrix4 Matrix4::Ortho(float Left, float Right, float Bottom, float Top) {
    
    Matrix4 Result = Matrix4(1);
    Result.col0.x = 2.0f / (Right - Left);
    Result.col1.y = 2.0f / (Top - Bottom);
    Result.col2.z = - 1.0f;
    Result.col3.x = - (Right + Left) / (Right - Left);
    Result.col3.y = - (Top + Bottom) / (Top - Bottom);
    
    return Result;
}

Matrix4 Matrix4::CreateFromAxisAngle(Vector3 Axis, float Angle) {
    Vector3 NormAxis = Normalize(Axis);
    float axisX = NormAxis.x;
    float axisY = NormAxis.y;
    float axisZ = NormAxis.z;

    float Cos = cos(-Angle);
    float Sin = sin(-Angle);
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

    Matrix4 Result = Matrix4();
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
    Result.col3 = Vector4(0, 0, 0, 1);

    return Result;
}

Matrix4 Matrix4::FromQuaternion(Quaternion quat) {
    Vector4 AxisAngle = Quaternion::ToAxisAngle(quat);
    return CreateFromAxisAngle(AxisAngle.Xyz(), AxisAngle.w);
}

Matrix4 Matrix4::operator*(const Matrix4& right) const {

    Vector4 SrcA0 = col0;
    Vector4 SrcA1 = col1;
    Vector4 SrcA2 = col2;
    Vector4 SrcA3 = col3;

    Vector4 SrcB0 = right.col0;
    Vector4 SrcB1 = right.col1;
    Vector4 SrcB2 = right.col2;
    Vector4 SrcB3 = right.col3;

    Matrix4 Result(0);
    Result.col0 = SrcA0 * SrcB0.x + SrcA1 * SrcB0.y + SrcA2 * SrcB0.z + SrcA3 * SrcB0.w;
    Result.col1 = SrcA0 * SrcB1.x + SrcA1 * SrcB1.y + SrcA2 * SrcB1.z + SrcA3 * SrcB1.w;
    Result.col2 = SrcA0 * SrcB2.x + SrcA1 * SrcB2.y + SrcA2 * SrcB2.z + SrcA3 * SrcB2.w;
    Result.col3 = SrcA0 * SrcB3.x + SrcA1 * SrcB3.y + SrcA2 * SrcB3.z + SrcA3 * SrcB3.w;
    return Result;
}

Matrix4 Matrix4::operator*(const float& right) const {

    return Matrix4(
        col0 * right,
        col1 * right,
        col2 * right,
        col3 * right
    );
}

Vector4 Matrix4::operator*(const Vector4& right) const {

    Vector4 Mov0(right.x);
    Vector4 Mov1(right.y);
    Vector4 Mul0 = col0 * Mov0;
    Vector4 Mul1 = col1 * Mov1;
    Vector4 Add0 = Mul0 + Mul1;

    Vector4 Mov2(right.z);
    Vector4 Mov3(right.w);
    Vector4 Mul2 = col2 * Mov2;
    Vector4 Mul3 = col3 * Mov3;
    Vector4 Add1 = Mul2 + Mul3;
    return Add0 + Add1;

}

Vector3 Matrix4::operator*(const Vector3& right) const {
    return (*this * Vector4(right)).Xyz();
}