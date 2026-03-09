#pragma once

#include <mars/math/vector3.hpp>
#include <mars/math/vector4.hpp>

#include <vector>

namespace mars::mesh::core {

struct triangle_adjacency {
	std::vector<unsigned int> counts;
	std::vector<unsigned int> offsets;
	std::vector<unsigned int> triangles;
};

struct surface_cone {
	mars::vector3<float> center;
	mars::vector3<float> axis;
};

struct kd_tree_node {
	union {
		float split_position;
		unsigned int primitive_index;
	};

	unsigned int split_axis : 2;
	unsigned int child_offset : 30;
};

struct simd_bvh_bounds {
	mars::vector4<float> min_corner;
	mars::vector4<float> max_corner;
};

struct bvh_bounds {
	mars::vector3<float> min_corner;
	mars::vector3<float> max_corner;
};

} // namespace mars::mesh::core
