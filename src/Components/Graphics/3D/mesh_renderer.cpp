#include "mesh_renderer.hpp"
#include "Math/matrix4.hpp"
#include "Math/matrix3.hpp"
#include "Math/vector3.hpp"
#include "Time/time_helper.hpp"
#include "Manager/light_manager.hpp"
#include "Manager/shader_manager.hpp"
#include "Manager/render_pass_manager.hpp"
#include "Manager/pipeline_manager.hpp"
#include "Components/Graphics/camera.hpp"
#include "Graphics/Renderer/deferred_renderer.hpp"

void mesh_renderer::load_mesh(const string& _mesh_location) {
    if (get_engine() == nullptr) {
        mesh_path = _mesh_location;
        return;
    }

    if (renderer_mesh == nullptr)
        renderer_mesh = new mesh();
    renderer_mesh->load(_mesh_location);

    if (vertice_buffer == nullptr) {
        vertice_buffer = new buffer(get_engine());
        vertice_buffer->create(renderer_mesh->vertices.size() * sizeof(vertex3), MEM_BUFF_VERTEX);
    }

    if (indice_buffer == nullptr) {
        indice_buffer = new buffer(get_engine());
        indice_buffer->create(renderer_mesh->indices.size() * sizeof(uint32_t), MEM_BUFF_INDEX);
    }

    //Need to resize verticeBuffer;
    vertice_buffer->update(renderer_mesh->vertices.data());
    indice_buffer->update(renderer_mesh->indices.data());
}

void mesh_renderer::load_texture(const std::string &_texture) {
    if (get_engine() == nullptr) {
        texture_path = _texture;
        return;
    }

    if (mesh_texture == nullptr)
        mesh_texture = new texture(get_engine());

    mesh_texture->load_texture(_texture);
}

void mesh_renderer::load() {
    model = { };

    is_renderer = true;
    if (!mesh_path.empty())
        load_mesh(mesh_path);

    if (!texture_path.empty())
        load_texture(texture_path);

    _shader = shader_manager::get_shader("Engine/Assets/Shaders/Model.shader", get_engine());

    _pipeline = pipeline_manager::get_pipeline("Model");

    if (_pipeline == nullptr) {
        auto desc = vertex3::get_description();
        _pipeline = new pipeline(get_engine(), _shader, render_pass_manager::get_render_pass("Renderer", get_engine()));
        _pipeline->create_layout();
        _pipeline->apply_input_description(&desc);
        _pipeline->create();
        pipeline_manager::add_pipeline("Model", _pipeline);
    }

    data = new shader_data(_shader, get_engine());
    data->get_uniform("Model")->generate(sizeof(shader_model));
    data->get_uniform("Material")->generate(sizeof(material_data));
    data->get_uniform("texCoord")->setTexture(mesh_texture);
    data->generate();

    get_command_buffer()->render_pass = render_pass_manager::get_render_pass("Renderer", get_engine());
}

void mesh_renderer::update() {
    if (vertice_buffer == nullptr)
        return;

    matrix4 pv = get_engine()->get_camera()->projection_view;
    matrix4 trans = get_transform()->get_transform();
    model.model = trans;
    model.mvp = pv * trans;

    data->get_uniform("Model")->update(&model);
    data->get_uniform("Material")->update(&get_material()->data);
}

void mesh_renderer::pre_render() {
    if (vertice_buffer == nullptr)
        return;

    _pipeline->bind(get_command_buffer());
    vertice_buffer->bind(get_command_buffer());
    indice_buffer->bind(get_command_buffer());
    data->bind(get_command_buffer(), _pipeline);
    get_command_buffer()->draw_indexed(renderer_mesh->indices.size(), 1);
}