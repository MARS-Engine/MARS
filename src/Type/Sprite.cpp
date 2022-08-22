#include "Sprite.hpp"

Sprite::Sprite(Texture* _texture, Rect _dimensions) {
    texture = _texture;
    dimensions = _dimensions;
}

Sprite::Sprite(Texture* _texture, Vector2 position, Vector2 size) {
    texture = _texture;
    dimensions = Rect(position, size);
}

Quad Sprite::GetUV() {
    return Quad{
        dimensions.position,
        dimensions.position + Vector2(dimensions.size.x, 0.0f),
        dimensions.position + Vector2(0.0f, dimensions.size.y),
        dimensions.position + dimensions.size,
    } / texture->GetSize();
}