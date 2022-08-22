#ifndef __RECT__
#define __RECT__

#include "Vector2.hpp"

struct Rect {
public:
    Vector2 position;
    Vector2 size;

    Rect();
    Rect(Vector2 position, Vector2 size);
    Rect(float x, float y, float width, float height);
};

#endif