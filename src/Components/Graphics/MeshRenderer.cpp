#include "MeshRenderer.hpp"
#include "Math/Vector4.hpp"
#include "Math/Matrix4.hpp"
#include "Math/Vector3.hpp"
#include "Time/TimeHelper.hpp"
#include "Graphics/Vulkan/VPipeline.hpp"
#include "Math/Quaternion.hpp"
#include <tiny_obj_loader.h>

struct MeshPushConstants {
    Vector4 data;
    Matrix4 render_matrix;
};

struct Mesh {
    vector<Vertex3> vertices;

    bool load_from_obj(const char* filename) {
        //attrib will contain the vertex arrays of the file
        tinyobj::attrib_t attrib;
        //shapes contains the info for each separate object in the file
        std::vector<tinyobj::shape_t> shapes;
        //materials contains the information about the material of each shape, but we won't use it.
        std::vector<tinyobj::material_t> materials;

        //error and warning output from the load function
        std::string warn;
        std::string err;

        //load the OBJ file
        tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, nullptr);
        //make sure to output the warnings to the console, in case there are issues with the file
        if (!warn.empty()) {
            std::cout << "WARN: " << warn << std::endl;
        }
        //if we have any error, print it to the console, and break the mesh loading.
        //This happens if the file can't be found or is malformed
        if (!err.empty()) {
            std::cerr << err << std::endl;
            return false;
        }

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

                //hardcode loading to triangles
                int fv = 3;

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                    //vertex position
                    tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                    tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                    tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                    //vertex normal
                    tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                    tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                    tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

                    //copy it into our vertex
                    Vertex3 new_vert;
                    new_vert.position.x = vx;
                    new_vert.position.y = vy;
                    new_vert.position.z = vz;

                    new_vert.normal.x = nx;
                    new_vert.normal.y = ny;
                    new_vert.normal.z = nz;

                    //we are setting the vertex color as the vertex normal. This is just for display purposes
                    new_vert.color = new_vert.normal;


                    vertices.push_back(new_vert);
                }
                index_offset += fv;
            }
        }

        return true;
    }

};

Mesh mesh;

void MeshRenderer::Load() {
    shader = new Shader(GetEngine());
    shader->LoadShader("Data/Shaders/Model.shader");

    mesh = Mesh();
    mesh.load_from_obj("Data/Meshes/Monkey.obj");

    auto desc = mesh.vertices[0].GetDescription();
    pipeline = new Pipeline(GetEngine());
    pipeline->CreateLayout(sizeof(MeshPushConstants));
    pipeline->Create(shader, &desc);

    buffer = new Buffer(GetEngine());
    buffer->Create(mesh.vertices.size() * sizeof(Vertex3), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    buffer->Update(mesh.vertices.data());
}
float x = 0;
Vector3 camPos = { 0.f, 0.f, -2.f };
void MeshRenderer::Render() {
    MeshPushConstants constants;
    x += TimeHelper::deltaTime;
    constants.render_matrix =   Matrix4::PerspectiveFov(90.f, 1700.f / 900.f, 0.1f, 200.f) *
                                Matrix4::Translate(Matrix4(1.f), camPos) *
                                Matrix4::FromQuaternion(Quaternion::EulerToQuaternion(Vector3(0.0f, x, 0.0f)));

    pipeline->Bind(GetEngine()->commandBuffer);
    buffer->Bind(GetEngine()->commandBuffer);
    pipeline->UpdateConstant(GetEngine()->commandBuffer, &constants);
    GetEngine()->commandBuffer->Draw(mesh.vertices.size(), 1);
}