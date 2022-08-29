#include "quad.hpp"

quad quad::operator/(vector2 value) const {
    return {
            top_left / value,
            top_right / value,
            bottom_left / value,
            bottom_right / value
    };
}