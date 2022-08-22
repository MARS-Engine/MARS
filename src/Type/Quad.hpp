#ifndef __QUAD__
#define __QUAD__

#include "Math/Vector2.hpp"

struct Quad {
public:
    Vector2 topL;
    Vector2 topR;
    Vector2 botL;
    Vector2 botR;

    Quad operator/(Vector2 value) const;
};

#endif