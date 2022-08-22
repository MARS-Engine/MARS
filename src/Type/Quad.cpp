#include "Quad.hpp"

Quad Quad::operator/(Vector2 value) const {
    return {
        topL / value,
        topR / value,
        botL / value,
        botR / value
    };
}