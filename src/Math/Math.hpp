#ifndef _MATH_
#define _MATH_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <algorithm>

#include "Vector2.hpp"

struct Vector3;
struct Vector4;
struct Quaternion;

#define SMALL_NUMBER (1.e-8f)

using namespace std;

unsigned short getShort(unsigned char* array, int offset);
unsigned int getInt32(unsigned char* array, int offset);
bool is_number(const string& s);

bool IsPointInsideRect(Vector2 Point, Vector4 Rectangle);

string GenerateRandomString(size_t Length );
float RandomNumber(float min, float max);

Vector3 Cross(const Vector3 Left, const Vector3 Right);
Vector3 Projection(const Vector3 Left, const Vector3 Right);
Vector3 Normalize(Vector3 Input);
Vector4 Normalize(const Vector4 Input, float Tolerance = SMALL_NUMBER);
Quaternion Normalize(Quaternion Input);
Vector3 RotateX(Vector3 Origin, Vector3 Point, float Deg);
Vector3 RotateY(Vector3 Origin, Vector3 Point, float Deg);
Vector3 RotateZ(Vector3 Origin, Vector3 Point, float Deg);
Vector3 Screen2World(Vector2 Screen);
Vector3 Abs(Vector3 Input);
Vector2 Abs(Vector2 Input);

float MaxDim(Vector3 Input);
float Dot(const Vector3 Left, const Vector3 Right);
float AngleBetween(const Vector3 Left, const Vector3 Right);
float Magnitude(const Vector3 Input);
float Radians(float Degrees);
float ReverseSqrt(float Input);
float Lerp(Vector3 Input);
float Mix(Vector3 Input);
float InvSqrt(float Input);

Vector3 operator*(const float& left, const Vector3& right);

#endif