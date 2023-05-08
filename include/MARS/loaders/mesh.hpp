#ifndef MARS_MESH_
#define MARS_MESH_

#include <vector>
#include <MARS/math/vector3.hpp>
#include <MARS/math/vector2.hpp>
#include <cfloat>
#include <cstdint>

namespace mars_loader {

    struct mesh_volume {
        mars_math::vector3<float> min_extent = {  1 << FLT_MANT_DIG,  1 << FLT_MANT_DIG,  1 << FLT_MANT_DIG };
        mars_math::vector3<float> max_extent = { -1 << FLT_MANT_DIG, -1 << FLT_MANT_DIG, -1 << FLT_MANT_DIG };
    };

    struct mesh_base {
    public:
        mesh_volume volume;
        mars_math::vector3<float> center;
    };

    template<typename T> struct mesh : public mesh_base {
    public:
        std::vector<uint32_t> indices;
        std::vector<T> vertices;
    };
}

#endif