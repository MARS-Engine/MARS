#include "Math.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Matrix4.hpp"
#include "Quaternion.hpp"
#include "math.h"

unsigned short getShort(unsigned char* array, int offset) {
    
    return (short)(((short)array[offset]) << 8) | array[offset + 1];
}

unsigned int getInt32(unsigned char* array, int offset) {
    
    return int((unsigned char)(array[offset])     << 24 |
               (unsigned char)(array[offset + 1]) << 16 |
               (unsigned char)(array[offset + 2]) << 8 |
               (unsigned char)(array[offset + 3]));
}

bool is_number(const string& s) {
    
    return !s.empty() && find_if(s.begin(), 
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

bool IsPointInsideRect(Vector2 Point, Vector4 Rectangle) {
    if (Point.x >= Rectangle.x && Point.x <= Rectangle.x + Rectangle.z && Point.y >= Rectangle.y && Point.y <= Rectangle.y + Rectangle.w)
        return true;
    return false;
}

string GenerateRandomString(size_t Length ) {
    auto randchar = []() -> char {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    string str(Length,0);
    generate_n( str.begin(), Length, randchar );
    return str;
}

float RandomNumber(float min, float max) { srand(static_cast<unsigned>(time(NULL))); return static_cast<float>(rand()) / static_cast <float> (RAND_MAX / max + min); }

Vector3 Cross(const Vector3 Left, const Vector3 Right) {

    return Vector3(Left.y * Right.z - Right.y * Left.z,
                   Left.z * Right.x - Right.z * Left.x,
                   Left.x * Right.y - Right.x * Left.y);
}

Vector3 Projection(const Vector3 Left, const Vector3 Right) {

    Vector3 Rightn = Right / Magnitude(Right);
    return (Rightn) * Dot(Left, Rightn);
}

Vector3 Normalize(Vector3 Input) {

    return Input * (1.0f / Input.Length());
}

Vector4 Normalize(const Vector4 Input, float Tolerance) {

    float SquareSum = Input.x * Input.x + Input.y * Input.y + Input.z + Input.z + Input.w * Input.w;

    if (SquareSum >= Tolerance)
        return Input * InvSqrt(SquareSum);
    else
        return Vector4();
}

Quaternion Normalize(Quaternion Input) {
    float Scale = 1.0f / Input.Length();
    return Quaternion(Input.quat.Xyz() * Scale, Input.quat.w * Scale);
}


Vector3 RotateX(Vector3 Origin, Vector3 Point, float Deg) {

    Vector3 x = Vector3(1, 0, 0);
    Vector3 y = Vector3(0, cos(Deg), sin(Deg));
    Vector3 z = Vector3(0, -sin(Deg), cos(Deg));

    return Vector3(
            Point.x * x.x + Point.y * y.x + Point.z * z.x + Origin.x,
            Point.x * x.y + Point.y * y.y + Point.z * z.y + Origin.y,
            Point.x * x.z + Point.y * y.z + Point.z * z.z + Origin.z
    );
}

Vector3 RotateY(Vector3 Origin, Vector3 Point, float Deg) {

    Vector3 x = Vector3(cos(Deg), 0, sin(Deg));
    Vector3 y = Vector3(0, 1, 0);
    Vector3 z = Vector3(-sin(Deg), 0, cos(Deg));

    return Vector3(
            Point.x * x.x + Point.y * y.x + Point.z * z.x + Origin.x,
            Point.x * x.y + Point.y * y.y + Point.z * z.y + Origin.y,
            Point.x * x.z + Point.y * y.z + Point.z * z.z + Origin.z
    );
}

Vector3 RotateZ(Vector3 Origin, Vector3 Point, float Deg) {

    Vector3 x = Vector3(cos(Deg), sin(Deg), 0);
    Vector3 y = Vector3(-sin(Deg), cos(Deg), 0);
    Vector3 z = Vector3(0, 0, 1);

    return Vector3(
            Point.x * x.x + Point.y * y.x + Point.z * z.x + Origin.x,
            Point.x * x.y + Point.y * y.y + Point.z * z.y + Origin.y,
            Point.x * x.z + Point.y * y.z + Point.z * z.z + Origin.z
    );
}

Vector3 Screen2World(Vector2 MousePosition) {
    //Vector2 Size = Window::GetSize();
    //Vector3 ray_nds = Vector3((2.0f * MousePosition.x) / Size.x - 1.0f, 1.0f - (2.0f * MousePosition.y) / Size.y, 1.0f);
    //Vector4 Ray_Clip = Vector4(ray_nds.Xy(), 1.0, 1.0);
    //Vector4 Ray_eye = Vector4((Matrix4::Inverse(Camera::Projection) * Ray_Clip).Xy(), 1.0, 0.0);
    //return Normalize((Matrix4::Inverse(Camera::View) * Ray_eye).Xyz());
}

Vector3 Abs(Vector3 Input) {
    return Vector3(abs(Input.x), abs(Input.y), abs(Input.z));
}

Vector2 Abs(Vector2 Input) {
    return Vector2(abs(Input.x), abs(Input.y));
}

float MaxDim(Vector3 Input) {
    return (Input.x > Input.y && Input.x > Input.z ? Input.x : (Input.y > Input.z ? Input.y : Input.z));
}

float Dot(const Vector3 Left, const Vector3 Right) {

    return Left.x * Right.x + Left.y * Right.y + Left.z * Right.z;
}

float AngleBetween(const Vector3 Left, const Vector3 Right) {

    return acosf((Dot(Left, Right)) / (Magnitude(Left) * Magnitude(Right)));
}

float Magnitude(const Vector3 Input) {

    return (sqrtf(powf(Input.x, 2) + powf(Input.y, 2) + powf(Input.z, 2)));
}

float Radians(float Degrees) {
    return Degrees * 0.01745329251994329576923690768489f;
}

float ReverseSqrt(float Input) {
    return 1.0f / sqrt(Input);
}

float Mix(Vector3 Input) {
    return Input.x * (1.0f - Input.z) + Input.y * Input.z;
}

float Lerp(Vector3 Input) {
    return Mix(Input);
}

float InvSqrt(float Input) {
    return 1.0f / sqrt(Input);
}

bool Equals(float a, float b, float tolerance) {
    return (a + tolerance >= b) && (a - tolerance <= b);
}

Vector3 operator*(const float& left, const Vector3& right) {
    
    return Vector3(right.x * left, right.y * left, right.z * left);
}