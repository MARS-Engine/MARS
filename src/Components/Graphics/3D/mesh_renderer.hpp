#ifndef __MESH__RENDERER__
#define __MESH__RENDERER__

#include <vector>

#include "Multithread/component.hpp"
#include "Graphics/pipeline.hpp"
#include "Graphics/shader.hpp"
#include "Graphics/buffer.hpp"
#include "Graphics/texture.hpp"
#include "Graphics/VertexType/vertex3.hpp"
#include "Graphics/shader_data.hpp"
#include "Loaders/mesh.hpp"



class mesh_renderer : public component<mesh_renderer> {
private:
    pipeline* _pipeline;
public:
    buffer* vertice_buffer;
    buffer* indice_buffer;
    shader_data* data;
    mesh* renderer_mesh;
    std::string mesh_path;
    shader_model model;

    void load_mesh(const std::string& _mesh_location);
    void load() override;
    void update() override;
    void pre_render() override;
};

#endif