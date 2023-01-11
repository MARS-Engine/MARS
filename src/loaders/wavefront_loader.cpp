#include <MARS/loaders/wavefront_loader.hpp>
#include <MARS/debug/debug.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>

bool mars_loader::wave_vertex::operator==(const mars_loader::wave_vertex& _right) const {
    return vertex == _right.vertex && uv == _right.uv;
}

bool mars_loader::wavefront_mesh::load_resource(const std::string &_path) {
    return wavefront_load(_path, this);
}

void mars_loader::wavefront_mesh::clean() {

}

bool mars_loader::wavefront_load(const std::string& _path, wavefront_mesh* _mesh) {
    tinyobj::attrib_t attrib;

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, _path.c_str(), "./", true)) {
        mars_debug::debug::error("MARS - Wavefront Loader - Failed to load object, path - " + _path);
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
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                },
                {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f -  attrib.texcoords[2 * index.texcoord_index + 1]
                }
            };

            //unorderd_map might be faster but im to lazy (and dumb) to add hash functions, parallel could also be used to make it even faster
            auto loc = std::find(_mesh->vertices.begin(), _mesh->vertices.end(), vertex);

            if (loc == _mesh->vertices.end()) {
                _mesh->indices.push_back(_mesh->vertices.size());
                _mesh->vertices.push_back(vertex);
            }
            else
                _mesh->indices.push_back(std::distance(_mesh->vertices.begin(), loc));
        }
    }

    return true;
}