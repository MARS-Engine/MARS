#include "sprite.hpp"

sprite::sprite(texture* _texture, rectangle _dimension) {
    sprite_texture = _texture;
    dimension = _dimension;
}

sprite::sprite(texture* _texture, vector2 position, vector2 size) {
    sprite_texture = _texture;
    dimension = rectangle(position, size);
}

quad sprite::get_uv() {
    return quad{
            dimension.position,
            dimension.position + vector2(dimension.size.x, 0.0f),
            dimension.position + vector2(0.0f, dimension.size.y),
            dimension.position + dimension.size,
    } / sprite_texture->get_size();
}