#include "sprite_renderer.hpp"
#include "Manager/shader_manager.hpp"
#include "Manager/pipeline_manager.hpp"
#include "Components/Graphics/camera.hpp"
#include "Manager/material_manager.hpp"
#include "Manager/light_manager.hpp"
#include "Math/matrix3.hpp"
#include "Time/time_helper.hpp"
#include "Manager/render_pass_manager.hpp"
#include <math.h>
#include "Graphics/Renderer/deferred_renderer.hpp"
#include "Graphics/window.hpp"

vector3 sprite_renderer::vertices[4] = {
        { -.5f,  .5f, .0f },
        {  .5f,  .5f, .0f },
        { -.5f, -.5f, .0f },
        {  .5f, -.5f, .0f }
};

int sprite_renderer::indices[6] = {0, 1, 2, 1, 3, 2 };

struct Model {
    matrix4 mvp;
    matrix4 model;
};

void sprite_renderer::load() {
    component::load();

    is_renderer = true;

    if (get_material()->name == "default") {
        set_material(material_manager::get_material("Sprite"));
        get_material()->enable_transparency = true;
        get_material()->mat_shader = shader_manager::get_shader("Engine/Assets/Shaders/Sprite.shader", get_engine());
    }

    vertice_buffer = new buffer(get_engine());
    vertice_buffer->create(sizeof(vertex3) * 4, MEM_BUFF_VERTEX);

    indice_buffer = new buffer(get_engine());
    indice_buffer->create(sizeof(uint32_t) * 6, MEM_BUFF_INDEX);

    vertice_buffer->update(&vertices);
    indice_buffer->update(&indices);

    _pipeline = pipeline_manager::get_pipeline("SpritePipeline" + get_material()->mat_shader->location);

    if (_pipeline == nullptr) {
        auto desc = vector3::get_description();
        _pipeline = new pipeline(get_engine(), get_material()->mat_shader, render_pass_manager::get_render_pass("Renderer",
                                                                                                               get_engine()));
        _pipeline->create_layout();
        _pipeline->apply_viewport({.size = get_engine()->surf_window->size});
        _pipeline->apply_input_description(&desc);
        _pipeline->create();
        pipeline_manager::add_pipeline("SpritePipeline" + get_material()->mat_shader->location, _pipeline);
    }

    data = new shader_data(get_material()->mat_shader, get_engine());
    data->get_uniform("UV")->generate(sizeof(quad));
    data->get_uniform("Model")->generate(sizeof(Model));
    data->get_uniform("texCoord")->setTexture(_sprite->sprite_texture);
    data->get_uniform("__SPRITE_RENDERER")->generate(sizeof(sprite_renderer_data));
    data->generate();

    _last_texture = _sprite->sprite_texture;

    set_sprite(_sprite);

    get_command_buffer()->render_pass = render_pass_manager::get_render_pass("Renderer", get_engine());
}

void sprite_renderer::set_sprite(sprite* _new_sprite) {
    _sprite = _new_sprite;
    if (!loaded)
        return;

    uv = _sprite->get_uv();

    if (_last_texture != _sprite->sprite_texture) {
        data->change_texture("texCoord", _sprite->sprite_texture);
        _last_texture = _sprite->sprite_texture;
        object->execute_code(PRE_RENDER);
    }

    data->get_uniform("UV")->update(&uv);
}

void sprite_renderer::update() {
    Model model;
    model.model = get_transform()->get_transform();
    model.mvp = get_engine()->get_camera()->projection_view * model.model;

    data->get_uniform("Model")->update(&model);
    data->get_uniform("__SPRITE_RENDERER")->update(&renderer_data);
}

void sprite_renderer::pre_render() {
    _pipeline->bind(get_command_buffer());
    vertice_buffer->bind(get_command_buffer());
    indice_buffer->bind(get_command_buffer());
    data->bind(get_command_buffer(), _pipeline);
    get_command_buffer()->draw_indexed(6, 1);
}