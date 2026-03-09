#pragma once

#include <mars/math/vector3.hpp>

#include <cstddef>
#include <span>
#include <vector>

namespace mars::mesh {

struct partition_clusters_options {
	std::span<const unsigned int> item_position_indices = {};
	std::span<const unsigned int> item_position_counts = {};
	std::span<const mars::vector3<float>> positions = {};
	size_t target_group_size = 8;
};

struct partition_clusters_result {
	std::vector<unsigned int> partitions;
	size_t partition_count = 0;
	bool improved = false;
};

partition_clusters_result partition_clusters(const partition_clusters_options& options);

} // namespace mars::mesh
