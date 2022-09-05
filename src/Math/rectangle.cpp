#include "rectangle.hpp"

rectangle::rectangle() {
    position = vector2();
    size = vector2();
}
rectangle::rectangle(vector2 _position, vector2 _size) {
    position = _position;
    size = _size;
}

rectangle::rectangle(float x, float y, float width, float height) {
    position = vector2(x, y);
    size = vector2(width, height);
}