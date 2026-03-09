#include <mars/mesh/mesh.hpp>
#include <mars/mesh/partition.hpp>

namespace mars::mesh {

partition_clusters_result partition_clusters(const partition_clusters_options& options) {
	partition_clusters_result result;
	const size_t item_count = options.item_position_counts.size();
	if (item_count == 0 || options.target_group_size == 0 || options.positions.empty())
		return result;

	result.partitions.resize(item_count, 0u);
	result.partition_count = mesh_partition_clusters(
	    result.partitions.data(),
	    options.item_position_indices.data(),
	    options.item_position_indices.size(),
	    options.item_position_counts.data(),
	    item_count,
	    &options.positions[0].x,
	    options.positions.size(),
	    sizeof(mars::vector3<float>),
	    options.target_group_size);
	result.improved = result.partition_count > 0 && result.partition_count < item_count;
	return result;
}

} // namespace mars::mesh
