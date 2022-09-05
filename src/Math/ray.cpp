#include "ray.hpp"

ray::ray() {
    origin = direction = vector3();
    get_intersection = false;
}

ray::ray(vector3 _origin, vector3 _direction) {

    origin = _origin;
    direction = _direction;
    get_intersection = false;
}

ray::ray(vector3 _origin, vector3 _direction, bool _get_intersection) {

    origin = _origin;
    direction = _direction;
    get_intersection = _get_intersection;
}

vector3 ray::get_point(float distance) {
    return origin + direction * distance;
}

bool ray::operator==(const ray& other) const {
    return origin == other.origin && direction == other.direction && get_intersection == other.get_intersection;
}

bool ray::operator<(const ray& other) const {
    return (origin.x < other.origin.x || origin.y < other.origin.y || origin.z < other.origin.z) && (direction.x < other.direction.x || direction.y < other.direction.y || direction.z < other.direction.z);
}