#pragma once

#include <mars/math/vector3.hpp>

#include <vector>

namespace mars::mesh::core {

struct cluster_adjacency {
	std::vector<unsigned int> offsets;
	std::vector<unsigned int> clusters;
	std::vector<unsigned int> shared;
};

struct cluster_partition_group {
	int group;
	int next;
	unsigned int size;
	unsigned int vertices;

	mars::vector3<float> center;
	float radius;
};

struct group_merge_candidate {
	unsigned int group_id;
	int priority;
};

} // namespace mars::mesh::core
