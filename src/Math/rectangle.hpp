#ifndef __RECT__
#define __RECT__

#include "vector2.hpp"

struct rectangle {
public:
    vector2 position;
    vector2 size;

    rectangle();
    rectangle(vector2 _position, vector2 _size);
    rectangle(float x, float y, float width, float height);
};

#endif