#include "math.hpp"
#include "vector3.hpp"
#include "vector4.hpp"
#include "matrix4.hpp"
#include "quaternion.hpp"
#include "math.h"

unsigned short get_short(unsigned char* array, int offset) {
    
    return (short)(((short)array[offset]) << 8) | array[offset + 1];
}

unsigned int get_int32(unsigned char* array, int offset) {
    
    return int((unsigned char)(array[offset])     << 24 |
               (unsigned char)(array[offset + 1]) << 16 |
               (unsigned char)(array[offset + 2]) << 8 |
               (unsigned char)(array[offset + 3]));
}

bool is_number(const std::string& s) {
    
    return !s.empty() && find_if(s.begin(), 
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

bool is_point_inside_rect(vector2 Point, vector4 Rectangle) {
    if (Point.x >= Rectangle.x && Point.x <= Rectangle.x + Rectangle.z && Point.y >= Rectangle.y && Point.y <= Rectangle.y + Rectangle.w)
        return true;
    return false;
}

std::string generate_random_string(size_t Length ) {
    auto randchar = []() -> char {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(Length,0);
    generate_n( str.begin(), Length, randchar );
    return str;
}

float random_number(float min, float max) { srand(static_cast<unsigned>(time(NULL))); return static_cast<float>(rand()) / static_cast <float> (RAND_MAX / max + min); }

vector3 cross(vector3 Left, vector3 Right) {

    return vector3(Left.y * Right.z - Right.y * Left.z,
                   Left.z * Right.x - Right.z * Left.x,
                   Left.x * Right.y - Right.x * Left.y);
}

vector3 normalize(vector3 Input) {

    return Input * (1.0f / Input.length());
}

vector4 normalize(vector4 Input, float Tolerance) {

    float SquareSum = Input.x * Input.x + Input.y * Input.y + Input.z + Input.z + Input.w * Input.w;

    if (SquareSum >= Tolerance)
        return Input * inv_sqrt(SquareSum);
    else
        return vector4();
}

quaternion normalize(quaternion Input) {
    float Scale = 1.0f / Input.length();
    return quaternion(Input.value.xyz() * Scale, Input.value.w * Scale);
}


vector3 rotateX(vector3 Origin, vector3 Point, float Deg) {

    vector3 x = vector3(1, 0, 0);
    vector3 y = vector3(0, cos(Deg), sin(Deg));
    vector3 z = vector3(0, -sin(Deg), cos(Deg));

    return vector3(
            Point.x * x.x + Point.y * y.x + Point.z * z.x + Origin.x,
            Point.x * x.y + Point.y * y.y + Point.z * z.y + Origin.y,
            Point.x * x.z + Point.y * y.z + Point.z * z.z + Origin.z
    );
}

vector3 rotateY(vector3 Origin, vector3 Point, float Deg) {

    vector3 x = vector3(cos(Deg), 0, sin(Deg));
    vector3 y = vector3(0, 1, 0);
    vector3 z = vector3(-sin(Deg), 0, cos(Deg));

    return vector3(
            Point.x * x.x + Point.y * y.x + Point.z * z.x + Origin.x,
            Point.x * x.y + Point.y * y.y + Point.z * z.y + Origin.y,
            Point.x * x.z + Point.y * y.z + Point.z * z.z + Origin.z
    );
}

vector3 rotateZ(vector3 Origin, vector3 Point, float Deg) {

    vector3 x = vector3(cos(Deg), sin(Deg), 0);
    vector3 y = vector3(-sin(Deg), cos(Deg), 0);
    vector3 z = vector3(0, 0, 1);

    return vector3(
            Point.x * x.x + Point.y * y.x + Point.z * z.x + Origin.x,
            Point.x * x.y + Point.y * y.y + Point.z * z.y + Origin.y,
            Point.x * x.z + Point.y * y.z + Point.z * z.z + Origin.z
    );
}

vector3 screen_to_world(vector2 Screen) {
    //Vector2 Size = Window::GetSize();
    //Vector3 ray_nds = Vector3((2.0f * MousePosition.x) / Size.x - 1.0f, 1.0f - (2.0f * MousePosition.y) / Size.y, 1.0f);
    //Vector4 Ray_Clip = Vector4(ray_nds.Xy(), 1.0, 1.0);
    //Vector4 Ray_eye = Vector4((Matrix4::Inverse(Camera::Projection) * Ray_Clip).Xy(), 1.0, 0.0);
    //return Normalize((Matrix4::Inverse(Camera::View) * Ray_eye).Xyz());
}

vector3 abs(vector3 Input) {
    return vector3(abs(Input.x), abs(Input.y), abs(Input.z));
}

vector2 abs(vector2 Input) {
    return vector2(abs(Input.x), abs(Input.y));
}

float max_dim(vector3 Input) {
    return (Input.x > Input.y && Input.x > Input.z ? Input.x : (Input.y > Input.z ? Input.y : Input.z));
}

float magnitude(vector3 Input) {

    return (sqrtf(powf(Input.x, 2) + powf(Input.y, 2) + powf(Input.z, 2)));
}

float radians(float Degrees) {
    return Degrees * 0.01745329251994329576923690768489f;
}

float reverse_sqrt(float Input) {
    return 1.0f / sqrt(Input);
}

float mix(vector3 Input) {
    return Input.x * (1.0f - Input.z) + Input.y * Input.z;
}

float lerp(vector3 Input) {
    return mix(Input);
}

float inv_sqrt(float Input) {
    return 1.0f / sqrt(Input);
}

bool equals(float a, float b, float tolerance) {
    return (a + tolerance >= b) && (a - tolerance <= b);
}

vector3 operator*(const float& left, const vector3& right) {
    
    return vector3(right.x * left, right.y * left, right.z * left);
}