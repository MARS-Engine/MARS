#ifndef __WAVEFRONT__LOADER__
#define __WAVEFRONT__LOADER__

#include <string>
#include "mesh.hpp"
#include <MVRE/resources/resource_manager.hpp>

namespace mvre_loader {
    struct wave_vertex {
        mvre_math::vector3<float> vertex;
        mvre_math::vector2<float> uv;
    };

    class wavefront_mesh : public mesh<wave_vertex>, public mvre_resources::resource_base {
    public:

        bool load_resource(const std::string &_path) override;
        void clean() override;
    };

    bool wavefront_load(const std::string& _path, wavefront_mesh* _mesh);
}

#endif