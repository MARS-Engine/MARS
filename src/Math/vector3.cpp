#include "vector3.hpp"
#include "quaternion.hpp"

#include <math.h>

vector3::vector3() {

    x = y = z = 0.0f;
}

vector3::vector3(float _x, float _y, float _z) {

    x = _x;
    y = _y;
    z = _z;
}

vector3 vector3::zero() {
    return {0.f, 0.f, 0.f};
}

vector3 vector3::one() {
    return {1.f, 1.f, 1.f};
}

vector3 vector3::up() {
    return {0.f, 1.f, 0.f};
}

vector3 vector3::forward() {
    return {0.f, 0.f, 1.f};
}

vector2 vector3::xy() {
    return {x, y};
}

float vector3::length() {
    return sqrt(x * x + y * y + z * z);
}

float vector3::length_squared() {
    return x * x + y * y + z * z;
}

vector3 vector3::get_safe_normal(float tolerance) {
    float SquareSum = x * x + y * y + z * z;

    if (SquareSum == 1.0f)
        return *this;
    else if (SquareSum < tolerance)
        return vector3();
    
    float Scale = inv_sqrt(SquareSum);
    return {x * Scale, y * Scale, z * Scale};
}

vector3 vector3::move_towards(vector3 current, vector3 target, float max_distance_delta) {

    vector3 Direction = target - current;
    float sqdist = Direction.x * Direction.x + Direction.y * Direction.y + Direction.z * Direction.z;

    if (sqdist == 0 || (max_distance_delta >= 0 && sqdist <= max_distance_delta * max_distance_delta))
        return target;

    float Dist = sqrt(sqdist);

    return current + Direction / Dist * max_distance_delta;
}

vector3 vector3::rotate_around(vector3 position, vector3 target, vector3 up, float angle) {

    quaternion Quat = quaternion(up, angle);
    vector3 Dif = position - target;
    Dif = Quat * Dif;
    return target + Dif;
}

float vector3::distance(vector3 origin, vector3 target) {
    return magnitude(origin - target);
}

float vector3::dot(vector3 Left, vector3 Right) {

    return Left.x * Right.x + Left.y * Right.y + Left.z * Right.z;
}

float vector3::angle_between(vector3 Left, vector3 Right) {

    return acosf((dot(Left, Right)) / (magnitude(Left) * magnitude(Right)));
}

vector3 vector3::projection(vector3 Left, vector3 Right) {

    vector3 Rightn = Right / magnitude(Right);
    return (Rightn) * dot(Left, Rightn);
}


bool vector3::operator==(const vector3& other) const {

    return x == other.x && y == other.y && z == other.z;
}

bool vector3::operator!=(const vector3& other) const {

    return !(x == other.x && y == other.y && z == other.z);
}


vector3 vector3::operator+(const vector3& right) const {

    return {x + right.x, y + right.y, z + right.z};
}

vector3 vector3::operator+(const vector2& right) const {

    return {x + right.x, y + right.y, z};
}

vector3 vector3::operator-(const vector3& right) const {

    return {x - right.x, y - right.y, z - right.z};
}

vector3 vector3::operator*(const vector3& right) const {

    return {x * right.x, y * right.y, z * right.z};
}

vector3 vector3::operator*(const float& right) const {

    return {x * right, y * right, z * right};
}

vector3 vector3::operator/(const vector3& right) const {

    return {x / right.x, y / right.y, z / right.z};
}

vector3 vector3::operator/(const float& right) const {

    return {x / right, y / right, z / right};
}

vector3& vector3::operator+=(const vector3& right) {

    x += right.x;
    y += right.y;
    z += right.z;
    return *this;
}

vector3& vector3::operator+=(const vector2& right) {

    x += right.x;
    y += right.y;
    return *this;
}

vector3& vector3::operator-=(const vector3& right) {

    x -= right.x;
    y -= right.y;
    z -= right.z;
    return *this;
}

float& vector3::operator[](int index) {

    switch (index) {
        case 0:
                return x;
        case 1:
                return y;
        case 2:
                return z;
        default:
            return x;
    }
}

size_t vector3::operator()(const vector3 &v) const {
    return ((hash<float>()(v.x) ^ (hash<float>()(v.y) << 1)) >> 1) ^ (hash<float>()(v.z) << 1);
}

VertexInputDescription vector3::get_description() {
    VertexInputDescription description;

    VkVertexInputBindingDescription mainBinding = {};
    mainBinding.binding = 0;
    mainBinding.stride = sizeof(vector3);
    mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    description.bindings.push_back(mainBinding);

    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = 0;

    description.attributes.push_back(positionAttribute);
    return description;
}