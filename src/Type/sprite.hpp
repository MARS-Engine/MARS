#ifndef __SPRITE__
#define __SPRITE__

#include "Graphics/texture.hpp"
#include "Math/rectangle.hpp"
#include "quad.hpp"

struct sprite {
public:
    texture* sprite_texture;
    rectangle dimension;

    sprite(texture* _texture, rectangle _dimension);
    sprite(texture* _texture, vector2 position, vector2 size);

    quad get_uv();
};

#endif