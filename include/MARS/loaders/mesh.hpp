#ifndef MARS_MESH_
#define MARS_MESH_

#include <vector>
#include <MARS/math/vector3.hpp>
#include <MARS/math/vector2.hpp>

namespace mars_loader {

    template<typename T> class mesh {
    public:
        std::vector<uint32_t> indices;
        std::vector<T> vertices;
    };
}

#endif