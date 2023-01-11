#ifndef WAVEFRONT_LOADER_
#define WAVEFRONT_LOADER_

#include <string>
#include "mesh.hpp"
#include <MARS/resources/resource_manager.hpp>

namespace mars_loader {
    struct wave_vertex {
        mars_math::vector3<float> vertex;
        mars_math::vector3<float> normal;
        mars_math::vector2<float> uv;

        bool operator==(const wave_vertex& _right) const;
    };

    class wavefront_mesh : public mesh<wave_vertex>, public mars_resources::resource_base {
    public:

        bool load_resource(const std::string &_path) override;
        void clean() override;
    };

    bool wavefront_load(const std::string& _path, wavefront_mesh* _mesh);
}

#endif