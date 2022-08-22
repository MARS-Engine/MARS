#include "Rect.hpp"

Rect::Rect() {
    position = Vector2();
    size = Vector2();
}
Rect::Rect(Vector2 _position, Vector2 _size) {
    position = _position;
    size = _size;
}

Rect::Rect(float x, float y, float width, float height) {
    position = Vector2(x, y);
    size = Vector2(width, height);
}