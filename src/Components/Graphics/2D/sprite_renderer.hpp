#ifndef __SPRITE__RENDERER__
#define __SPRITE__RENDERER__

#include <string>

#include "Multithread/component.hpp"
#include "Graphics/VertexType/vertex3.hpp"
#include "Graphics/buffer.hpp"
#include "Graphics/texture.hpp"
#include "Graphics/shader.hpp"
#include "Graphics/pipeline.hpp"
#include "Graphics/shader_data.hpp"
#include "Type/sprite.hpp"

struct sprite_renderer_data {
    unsigned int flip_x;
    unsigned int flip_y;
};

class sprite_renderer : public component<sprite_renderer> {
private:
    texture* _last_texture = nullptr;
    sprite* _sprite = nullptr;
    pipeline* _pipeline = nullptr;
public:
    static vector3 vertices[4];
    static int indices[6];

    buffer* vertice_buffer = nullptr;
    buffer* indice_buffer = nullptr;
    shader_data* data = nullptr;
    quad uv;
    sprite_renderer_data renderer_data{};

    void set_sprite(sprite* _new_sprite);
    void load() override;
    void update() override;
    void pre_render() override;
};

#endif