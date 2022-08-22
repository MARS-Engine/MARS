#ifndef __SPRITE__
#define __SPRITE__

#include "Graphics/Texture.hpp"
#include "Math/Rect.hpp"
#include "Quad.hpp"

struct Sprite {
public:
    Texture* texture;
    Rect dimensions;

    Sprite(Texture* texture, Rect dimensions);
    Sprite(Texture* texture, Vector2 position, Vector2 size);

    Quad GetUV();
};

#endif