#include <MVRE/loaders/wavefront_loader.hpp>
#include <MVRE/debug/debug.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

bool mvre_loader::wavefront_mesh::load_resource(const std::string &_path) {
    return wavefront_load(_path, this);
}

void mvre_loader::wavefront_mesh::clean() {

}

bool mvre_loader::wavefront_load(const std::string& _path, wavefront_mesh* _mesh) {
    tinyobj::attrib_t attrib;

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, _path.c_str())) {
        mvre_debug::debug::error("MVRE - Wavefront Loader - Failed to load object, path - " + _path);
        return false;
    }

    for (const auto& shape: shapes) {
        for (auto index : shape.mesh.indices) {
            wave_vertex vertex{
                {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                },
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                }
            };

            _mesh->vertices.push_back(vertex);
            _mesh->indices.push_back(_mesh->indices.size());
        }
    }

    return true;
}