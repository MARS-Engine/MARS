#ifndef _RAY_
#define _RAY_

#include "vector3.hpp"

struct ray {
    vector3 origin;
    vector3 direction;
    bool get_intersection;

    ray();
    ray(vector3 _origin, vector3 _direction);
    ray(vector3 _origin, vector3 _direction, bool _get_intersection);

    vector3 get_point(float distance);
    bool operator==(const ray& other) const;
    bool operator<(const ray& other) const;
};

#endif