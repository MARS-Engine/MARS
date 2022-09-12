#ifndef _MATH_
#define _MATH_

#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <algorithm>
#include <math.h>
#include "vector2.hpp"

struct vector3;
struct vector4;
struct quaternion;

#define SMALL_NUMBER (1.e-8f)
#define rad2deg (180.0f/(float)M_PI)
#define deg2rad ((float)M_PI/180.0f)



unsigned short get_short(unsigned char* array, int offset);
unsigned int get_int32(unsigned char* array, int offset);
bool is_number(const std::string& s);

bool is_point_inside_rect(vector2 Point, vector4 Rectangle);

std::string generate_random_string(size_t Length );
float random_number(float min, float max);

vector3 cross(vector3 Left, vector3 Right);
vector3 normalize(vector3 Input);
vector4 normalize(vector4 Input, float Tolerance = SMALL_NUMBER);
quaternion normalize(quaternion Input);
vector3 rotateX(vector3 Origin, vector3 Point, float Deg);
vector3 rotateY(vector3 Origin, vector3 Point, float Deg);
vector3 rotateZ(vector3 Origin, vector3 Point, float Deg);
vector3 screen_to_world(vector2 Screen);
vector3 abs(vector3 Input);
vector2 abs(vector2 Input);

float max_dim(vector3 Input);
float magnitude(vector3 Input);
float radians(float Degrees);
float reverse_sqrt(float Input);
float lerp(vector3 Input);
float mix(vector3 Input);
float inv_sqrt(float Input);
bool equals(float a, float b, float tolerance);

vector3 operator*(const float& left, const vector3& right);

#endif