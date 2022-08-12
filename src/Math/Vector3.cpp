#include "Vector3.hpp"
#include "Quaternion.hpp"

#include <math.h>

Vector3::Vector3() {

    x = y = z = 0.0f;
}

Vector3::Vector3(float _x, float _y, float _z) {

    x = _x;
    y = _y;
    z = _z;
}

Vector3 Vector3::Zero() {
    return {0.f, 0.f, 0.f};
}

Vector3 Vector3::One() {
    return {1.f, 1.f, 1.f};
}

Vector3 Vector3::Up() {
    return {0.f, 1.f, 0.f};
}

Vector3 Vector3::Forward() {
    return {0.f, 0.f, 1.f};
}

Vector2 Vector3::Xy() {
    return {x, y};
}

float Vector3::Length() {
    return sqrt(x * x + y * y + z * z);
}

float Vector3::LengthSquared() {
    return x * x + y * y + z * z;
}

Vector3 Vector3::GetSafeNormal(float Tolerance) {
    float SquareSum = x * x + y * y + z * z;

    if (SquareSum == 1.0f)
        return *this;
    else if (SquareSum < Tolerance)
        return Vector3::Zero();
    
    float Scale = InvSqrt(SquareSum);
    return {x * Scale, y * Scale, z * Scale};
}

Vector3 Vector3::MoveTowards(Vector3 Current, Vector3 Target, float MaxDistanceDelta) {

    Vector3 Direction = Target - Current;
    float sqdist = Direction.x * Direction.x + Direction.y * Direction.y + Direction.z * Direction.z;

    if (sqdist == 0 || (MaxDistanceDelta >= 0 && sqdist <= MaxDistanceDelta * MaxDistanceDelta))
        return Target;

    float Dist = sqrt(sqdist);

    return Current + Direction / Dist * MaxDistanceDelta;
}

Vector3 Vector3::RotateAround(Vector3 Position, Vector3 Target, Vector3 Up, float Angle) {

    Quaternion Quat = Quaternion::FromAxisAngle(Up, Angle);
    Vector3 Dif = Position - Target;
    Dif = Quat * Dif;
    return Target + Dif;
}

bool Vector3::operator==(const Vector3& other) const {

    return x == other.x && y == other.y && z == other.z;
}

bool Vector3::operator!=(const Vector3& other) const {

    return !(x == other.x && y == other.y && z == other.z);
}


Vector3 Vector3::operator+(const Vector3& right) const {

    return {x + right.x, y + right.y, z + right.z};
}

Vector3 Vector3::operator+(const Vector2& right) const {

    return {x + right.x, y + right.y, z};
}

Vector3 Vector3::operator-(const Vector3& right) const {

    return {x - right.x, y - right.y, z - right.z};
}

Vector3 Vector3::operator*(const Vector3& right) const {

    return {x * right.x, y * right.y, z * right.z};
}

Vector3 Vector3::operator*(const float& right) const {

    return {x * right, y * right, z * right};
}

Vector3 Vector3::operator/(const Vector3& right) const {

    return {x / right.x, y / right.y, z / right.z};
}

Vector3 Vector3::operator/(const float& right) const {

    return {x / right, y / right, z / right};
}

Vector3& Vector3::operator+=(const Vector3& right) {

    x += right.x;
    y += right.y;
    z += right.z;
    return *this;
}

Vector3& Vector3::operator+=(const Vector2& right) {

    x += right.x;
    y += right.y;
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& right) {

    x -= right.x;
    y -= right.y;
    z -= right.z;
    return *this;
}

float& Vector3::operator[](int Index) {

    switch (Index) {
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

size_t Vector3::operator()(const Vector3 &v) const {
    return ((hash<float>()(v.x) ^ (hash<float>()(v.y) << 1)) >> 1) ^ (hash<float>()(v.z) << 1);
}

VertexInputDescription Vector3::GetDescription() {
    VertexInputDescription description;

    VkVertexInputBindingDescription mainBinding = {};
    mainBinding.binding = 0;
    mainBinding.stride = sizeof(Vector3);
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