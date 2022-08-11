#include "Mesh.hpp"
#include "Graphics/VertexType/Hash.hpp"
#include <tiny_obj_loader.h>

void Mesh::Load(const std::string& meshLocation) {
    vertices.resize(0);
    indices.resize(0);

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, meshLocation.c_str(), nullptr);

    if (!warn.empty())
        Debug::Alert("Mesh Warning - " + warn);

    if (!err.empty())
        Debug::Alert("Mesh Error - " + err);

    for (auto shape : shapes) {
        unordered_map<Vertex3, uint32_t> uniqueVertices{};

        for (auto indice : shape.mesh.indices) {
            Vertex3 vertice{};

            vertice.position = {
                    attrib.vertices[3 * indice.vertex_index],
                    attrib.vertices[3 * indice.vertex_index + 1],
                    attrib.vertices[3 * indice.vertex_index + 2]
            };

            vertice.uv = {
                    attrib.texcoords[2 * indice.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * indice.texcoord_index + 1],
            };

            vertice.normal = {
                    attrib.normals[3 * indice.normal_index + 0],
                    attrib.normals[3 * indice.normal_index + 1],
                    attrib.normals[3 * indice.normal_index + 2]
            };

            if (uniqueVertices.count(vertice) == 0) {
                uniqueVertices[vertice] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertice);
            }
            indices.push_back(uniqueVertices[vertice]);
        }
    }
}