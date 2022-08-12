#ifndef _MATRIX4_
#define _MATRIX4_

#include "Vector4.hpp"
#include "Math.hpp"
#include "Debug/Debug.hpp"

struct Matrix3;

struct Matrix4 {
    
    Vector4 col0;
    Vector4 col1;
    Vector4 col2;
    Vector4 col3;

    Matrix4();
    Matrix4(float V);
    Matrix4(Matrix3 V);
    Matrix4(Vector4 c0, Vector4 c1, Vector4 c2, Vector4 c3);

    static Matrix4 Translate(Vector3 translation);
    static Matrix4 Translate(Matrix4 Target, Vector3 translation);
    static Matrix4 Scale(Vector3 scale);
    static Matrix4 Scale(Matrix4 Target, Vector3 scale);
    static Matrix4 LookAtLH(Vector3 Eye, Vector3 Center, Vector3 Up);
    static Matrix4 LookAt(Vector3 Eye, Vector3 Center, Vector3 Up);
    static Matrix4 PerspectiveFovLH(float Fov, float Width, float Height, float zNear, float zFar);
    static Matrix4 PerspectiveFov(float Fov, float Aspect, float zNear, float zFar);
    static Matrix4 Inverse(Matrix4 Value);
    static Matrix4 InverseTranspose(Matrix4 Value);
    static Matrix4 Ortho(float Left, float Right, float Bottom, float Top);
    static Matrix4 FromQuaternion(Quaternion quat);
    static Matrix4 CreateFromAxisAngle(Vector3 Axis, float Angle);

    Matrix4 operator*(const Matrix4& right) const;
    Matrix4 operator*(const float& right) const;
    Vector4 operator*(const Vector4& right) const;
    Vector3 operator*(const Vector3& right) const;
    Matrix4 operator/=(const Matrix4& right);
    Matrix4 operator*=(const Matrix4& right);
    Matrix4 operator=(const Matrix4& right);
    Vector4& operator[](int index);
};


#endif