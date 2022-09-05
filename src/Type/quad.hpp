#ifndef __QUAD__
#define __QUAD__

#include "Math/vector2.hpp"

struct quad {
public:
    vector2 top_left;
    vector2 top_right;
    vector2 bottom_left;
    vector2 bottom_right;

    quad operator/(vector2 value) const;
};

#endif