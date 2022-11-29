#ifndef __MVRE__MESH__
#define __MVRE__MESH__

#include <vector>
#include <MVRE/math/vector3.hpp>
#include <MVRE/math/vector2.hpp>

namespace mvre_loader {

    template<typename T> class mesh {
    public:
        std::vector<uint32_t> indices;
        std::vector<T> vertices;
    };
}

#endif