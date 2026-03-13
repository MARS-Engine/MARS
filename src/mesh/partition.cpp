#include <mars/mesh/detail/partition_types.hpp>
#include <mars/mesh/mesh.hpp>

#include <assert.h>
#include <math.h>
#include <string.h>

namespace mars::mesh {

namespace core {

const int k_merge_depth_cutoff = 40;

static void filter_cluster_indices(unsigned int* unique_cluster_indices, unsigned int* cluster_offsets, const unsigned int* cluster_indices, const unsigned int* cluster_index_counts, size_t cluster_count, unsigned char* used_vertices, size_t vertex_count, size_t total_index_count) {
	(void)vertex_count;
	(void)total_index_count;

	size_t source_index_offset = 0;
	size_t write_offset = 0;

	for (size_t i = 0; i < cluster_count; ++i) {
		cluster_offsets[i] = unsigned(write_offset);

		for (size_t j = 0; j < cluster_index_counts[i]; ++j) {
			unsigned int vertex_index = cluster_indices[source_index_offset + j];
			assert(vertex_index < vertex_count);

			unique_cluster_indices[write_offset] = vertex_index;
			write_offset += 1 - used_vertices[vertex_index];
			used_vertices[vertex_index] = 1;
		}

		for (size_t j = cluster_offsets[i]; j < write_offset; ++j)
			used_vertices[unique_cluster_indices[j]] = 0;

		source_index_offset += cluster_index_counts[i];
	}

	assert(source_index_offset == total_index_count);
	assert(write_offset <= total_index_count);
	cluster_offsets[cluster_count] = unsigned(write_offset);
}

static float compute_cluster_bounds(const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_positions_stride, mars::vector3<float>* out_center) {
	size_t vertex_stride_float = vertex_positions_stride / sizeof(float);

	mars::vector3<float> center = {0, 0, 0};

	for (size_t j = 0; j < index_count; ++j) {
		const float* position = vertex_positions + indices[j] * vertex_stride_float;

		center.x += position[0];
		center.y += position[1];
		center.z += position[2];
	}

	if (index_count) {
		center.x /= float(index_count);
		center.y /= float(index_count);
		center.z /= float(index_count);
	}

	float radiussq = 0;

	for (size_t j = 0; j < index_count; ++j) {
		const float* position = vertex_positions + indices[j] * vertex_stride_float;

		float distance_sq = (position[0] - center.x) * (position[0] - center.x) + (position[1] - center.y) * (position[1] - center.y) + (position[2] - center.z) * (position[2] - center.z);

		radiussq = radiussq < distance_sq ? distance_sq : radiussq;
	}

	*out_center = center;
	return sqrtf(radiussq);
}

static void build_cluster_adjacency(cluster_adjacency& adjacency, const unsigned int* cluster_indices, const unsigned int* cluster_offsets, size_t cluster_count, size_t vertex_count, array_stack_heap_allocator<24>& allocator) {
	unsigned int* vertex_reference_offsets = allocator.allocate<unsigned int>(vertex_count + 1);

	memset(vertex_reference_offsets, 0, vertex_count * sizeof(unsigned int));

	for (size_t i = 0; i < cluster_count; ++i)
		for (size_t j = cluster_offsets[i]; j < cluster_offsets[i + 1]; ++j)
			vertex_reference_offsets[cluster_indices[j]]++;

	size_t total_adjacency = 0;

	for (size_t i = 0; i < cluster_count; ++i) {
		size_t count = 0;

		for (size_t j = cluster_offsets[i]; j < cluster_offsets[i + 1]; ++j)
			count += vertex_reference_offsets[cluster_indices[j]] - 1;

		total_adjacency += count < cluster_count - 1 ? count : cluster_count - 1;
	}

	adjacency.offsets.resize(cluster_count + 1);
	adjacency.clusters.resize(total_adjacency);
	adjacency.shared.resize(total_adjacency);

	size_t total_refs = 0;

	for (size_t i = 0; i < vertex_count; ++i) {
		size_t vertex_reference_count = vertex_reference_offsets[i];
		vertex_reference_offsets[i] = unsigned(total_refs);
		total_refs += vertex_reference_count;
	}

	unsigned int* vertex_reference_data = allocator.allocate<unsigned int>(total_refs);

	for (size_t i = 0; i < cluster_count; ++i)
		for (size_t j = cluster_offsets[i]; j < cluster_offsets[i + 1]; ++j)
			vertex_reference_data[vertex_reference_offsets[cluster_indices[j]]++] = unsigned(i);

	memmove(vertex_reference_offsets + 1, vertex_reference_offsets, vertex_count * sizeof(unsigned int));
	vertex_reference_offsets[0] = 0;

	adjacency.offsets[0] = 0;

	for (size_t i = 0; i < cluster_count; ++i) {
		unsigned int* adjacent_clusters = adjacency.clusters.data() + adjacency.offsets[i];
		unsigned int* shared_vertex_counts = adjacency.shared.data() + adjacency.offsets[i];
		size_t adjacency_count = 0;

		for (size_t j = cluster_offsets[i]; j < cluster_offsets[i + 1]; ++j) {
			unsigned int vertex_index = cluster_indices[j];

			for (size_t k = vertex_reference_offsets[vertex_index]; k < vertex_reference_offsets[vertex_index + 1]; ++k) {
				unsigned int adjacent_cluster_index = vertex_reference_data[k];
				assert(adjacent_cluster_index < cluster_count);

				if (adjacent_cluster_index == unsigned(i))
					continue;

				bool found = false;
				for (size_t existing_index = 0; existing_index < adjacency_count; ++existing_index)
					if (adjacent_clusters[existing_index] == adjacent_cluster_index) {
						found = true;
						shared_vertex_counts[existing_index]++;
						break;
					}

				if (!found) {
					adjacent_clusters[adjacency_count] = adjacent_cluster_index;
					shared_vertex_counts[adjacency_count] = 1;
					adjacency_count++;
				}
			}
		}

		adjacency.offsets[i + 1] = adjacency.offsets[i] + unsigned(adjacency_count);
	}

	assert(adjacency.offsets[cluster_count] <= total_adjacency);

	allocator.deallocate(vertex_reference_data);
}

static void heap_push(group_merge_candidate* heap, size_t size, group_merge_candidate item) {
	heap[size++] = item;

	size_t current_index = size - 1;
	while (current_index > 0 && heap[current_index].priority < heap[(current_index - 1) / 2].priority) {
		size_t parent_index = (current_index - 1) / 2;

		group_merge_candidate candidate = heap[current_index];
		heap[current_index] = heap[parent_index];
		heap[parent_index] = candidate;
		current_index = parent_index;
	}
}

static group_merge_candidate heap_pop(group_merge_candidate* heap, size_t size) {
	assert(size > 0);
	group_merge_candidate top = heap[0];

	heap[0] = heap[--size];

	size_t current_index = 0;
	while (current_index * 2 + 1 < size) {
		size_t child_index = current_index * 2 + 1;
		child_index += (child_index + 1 < size && heap[child_index + 1].priority < heap[child_index].priority);

		if (heap[child_index].priority >= heap[current_index].priority)
			break;

		group_merge_candidate candidate = heap[current_index];
		heap[current_index] = heap[child_index];
		heap[child_index] = candidate;
		current_index = child_index;
	}

	return top;
}

static unsigned int count_shared(const cluster_partition_group* groups, int first_group, int second_group, const cluster_adjacency& adjacency) {
	unsigned int total = 0;

	for (int first_cluster = first_group; first_cluster >= 0; first_cluster = groups[first_cluster].next)
		for (int second_cluster = second_group; second_cluster >= 0; second_cluster = groups[second_cluster].next) {
			for (unsigned int adjacency_index = adjacency.offsets[first_cluster]; adjacency_index < adjacency.offsets[first_cluster + 1]; ++adjacency_index)
				if (adjacency.clusters[adjacency_index] == unsigned(second_cluster)) {
					total += adjacency.shared[adjacency_index];
					break;
				}
		}

	return total;
}

static void merge_bounds(cluster_partition_group& target, const cluster_partition_group& source) {
	float target_radius = target.radius;
	float source_radius = source.radius;
	float delta_x = source.center.x - target.center.x;
	float delta_y = source.center.y - target.center.y;
	float delta_z = source.center.z - target.center.z;
	float center_distance = sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);

	if (center_distance + target_radius < source_radius) {
		target.center = source.center;
		target.radius = source.radius;
		return;
	}

	if (center_distance + source_radius > target_radius) {
		float interpolation = center_distance > 0 ? (center_distance + source_radius - target_radius) / (2 * center_distance) : 0.f;

		target.center.x += delta_x * interpolation;
		target.center.y += delta_y * interpolation;
		target.center.z += delta_z * interpolation;
		target.radius = (center_distance + source_radius + target_radius) / 2;
	}
}

static float bounds_score(const cluster_partition_group& target, const cluster_partition_group& source) {
	float target_radius = target.radius;
	float source_radius = source.radius;
	float delta_x = source.center.x - target.center.x;
	float delta_y = source.center.y - target.center.y;
	float delta_z = source.center.z - target.center.z;
	float center_distance = sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);

	float merged_radius = center_distance + target_radius < source_radius ? source_radius : (center_distance + source_radius < target_radius ? target_radius : (center_distance + source_radius + target_radius) / 2);

	return merged_radius > 0 ? target_radius / merged_radius : 0.f;
}

static int pick_group_to_merge(const cluster_partition_group* groups, int group_id, const cluster_adjacency& adjacency, size_t max_partition_size, bool use_bounds) {
	assert(groups[group_id].size > 0);

	float source_inverse_radius = 1.f / sqrtf(float(int(groups[group_id].vertices)));

	int best_group = -1;
	float best_score = 0;

	for (int cluster_index = group_id; cluster_index >= 0; cluster_index = groups[cluster_index].next) {
		for (unsigned int adjacency_index = adjacency.offsets[cluster_index]; adjacency_index != adjacency.offsets[cluster_index + 1]; ++adjacency_index) {
			int candidate_group = groups[adjacency.clusters[adjacency_index]].group;
			if (candidate_group < 0)
				continue;

			assert(groups[candidate_group].size > 0);
			if (groups[group_id].size + groups[candidate_group].size > max_partition_size)
				continue;

			unsigned int shared_vertices = count_shared(groups, group_id, candidate_group, adjacency);
			float candidate_inverse_radius = 1.f / sqrtf(float(int(groups[candidate_group].vertices)));

			float score = float(int(shared_vertices)) * (source_inverse_radius + candidate_inverse_radius);

			if (use_bounds)
				score *= 1.f + 0.4f * bounds_score(groups[group_id], groups[candidate_group]);

			if (score > best_score) {
				best_group = candidate_group;
				best_score = score;
			}
		}
	}

	return best_group;
}

static void merge_leaf(cluster_partition_group* groups, unsigned int* order, size_t count, size_t target_partition_size, size_t max_partition_size) {
	for (size_t i = 0; i < count; ++i) {
		unsigned int group_id = order[i];
		if (groups[group_id].size == 0 || groups[group_id].size >= target_partition_size)
			continue;

		float best_score = -1.f;
		int best_group = -1;

		for (size_t j = 0; j < count; ++j) {
			unsigned int candidate_group = order[j];
			if (group_id == candidate_group || groups[candidate_group].size == 0)
				continue;

			if (groups[group_id].size + groups[candidate_group].size > max_partition_size)
				continue;

			float score = bounds_score(groups[group_id], groups[candidate_group]);

			if (score > best_score) {
				best_score = score;
				best_group = candidate_group;
			}
		}

		if (best_group != -1) {
			unsigned int tail_group = best_group;
			while (groups[tail_group].next >= 0)
				tail_group = groups[tail_group].next;

			groups[tail_group].next = group_id;

			groups[best_group].size += groups[group_id].size;
			groups[group_id].size = 0;

			merge_bounds(groups[best_group], groups[group_id]);
			groups[group_id].radius = 0.f;
		}
	}
}

static size_t merge_partition(unsigned int* order, size_t count, const cluster_partition_group* groups, int axis, float pivot) {
	size_t write_offset = 0;

	for (size_t i = 0; i < count; ++i) {
		float coordinate = groups[order[i]].center[axis];

		unsigned int swapped_group = order[write_offset];
		order[write_offset] = order[i];
		order[i] = swapped_group;

		write_offset += coordinate < pivot;
	}

	return write_offset;
}

static void merge_spatial(cluster_partition_group* groups, unsigned int* order, size_t count, size_t target_partition_size, size_t max_partition_size, size_t leaf_size, int depth) {
	size_t total = 0;
	for (size_t i = 0; i < count; ++i)
		total += groups[order[i]].size;

	if (total <= max_partition_size || count <= leaf_size)
		return merge_leaf(groups, order, count, target_partition_size, max_partition_size);

	float mean[3] = {};
	float variance[3] = {};
	float running_count = 1;
	float inverse_running_count = 1;

	for (size_t i = 0; i < count; ++i, running_count += 1.f, inverse_running_count = 1.f / running_count) {
		const mars::vector3<float>& point = groups[order[i]].center;

		for (int k = 0; k < 3; ++k) {
			float delta = point[k] - mean[k];
			mean[k] += delta * inverse_running_count;
			variance[k] += delta * (point[k] - mean[k]);
		}
	}

	int split_axis = (variance[0] >= variance[1] && variance[0] >= variance[2]) ? 0 : (variance[1] >= variance[2] ? 1 : 2);

	float split_position = mean[split_axis];
	size_t middle = merge_partition(order, count, groups, split_axis, split_position);

	if (middle <= leaf_size / 2 || count - middle <= leaf_size / 2 || depth >= k_merge_depth_cutoff)
		middle = count / 2;

	merge_spatial(groups, order, middle, target_partition_size, max_partition_size, leaf_size, depth + 1);
	merge_spatial(groups, order + middle, count - middle, target_partition_size, max_partition_size, leaf_size, depth + 1);
}

} // namespace core

size_t mesh_partition_clusters(unsigned int* destination, const unsigned int* cluster_indices, size_t total_index_count, const unsigned int* cluster_index_counts, size_t cluster_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, size_t target_partition_size) {
	using namespace mars::mesh::core;

	assert((vertex_positions == NULL || vertex_positions_stride >= 12) && vertex_positions_stride <= 256);
	assert(vertex_positions_stride % sizeof(float) == 0);
	assert(target_partition_size > 0);

	size_t max_partition_size = target_partition_size + target_partition_size / 3;

	array_stack_heap_allocator<24> allocator;

	unsigned char* used = allocator.allocate<unsigned char>(vertex_count);
	memset(used, 0, vertex_count);

	unsigned int* cluster_newindices = allocator.allocate<unsigned int>(total_index_count);
	unsigned int* cluster_offsets = allocator.allocate<unsigned int>(cluster_count + 1);

	filter_cluster_indices(cluster_newindices, cluster_offsets, cluster_indices, cluster_index_counts, cluster_count, used, vertex_count, total_index_count);
	cluster_indices = cluster_newindices;

	cluster_adjacency adjacency = {};
	build_cluster_adjacency(adjacency, cluster_indices, cluster_offsets, cluster_count, vertex_count, allocator);

	cluster_partition_group* groups = allocator.allocate<cluster_partition_group>(cluster_count);
	memset(groups, 0, sizeof(cluster_partition_group) * cluster_count);

	group_merge_candidate* order = allocator.allocate<group_merge_candidate>(cluster_count);
	size_t pending = 0;

	for (size_t i = 0; i < cluster_count; ++i) {
		groups[i].group = int(i);
		groups[i].next = -1;
		groups[i].size = 1;
		groups[i].vertices = cluster_offsets[i + 1] - cluster_offsets[i];
		assert(groups[i].vertices > 0);

		if (vertex_positions)
			groups[i].radius = compute_cluster_bounds(cluster_indices + cluster_offsets[i], cluster_offsets[i + 1] - cluster_offsets[i], vertex_positions, vertex_positions_stride, &groups[i].center);

		group_merge_candidate item = {};
		item.group_id = unsigned(i);
		item.priority = groups[i].vertices;

		heap_push(order, pending++, item);
	}

	while (pending) {
		group_merge_candidate top = heap_pop(order, pending--);

		if (groups[top.group_id].size == 0)
			continue;

		for (int i = top.group_id; i >= 0; i = groups[i].next) {
			assert(groups[i].group == int(top.group_id));
			groups[i].group = -1;
		}

		if (groups[top.group_id].size >= target_partition_size)
			continue;

		int best_group = pick_group_to_merge(groups, top.group_id, adjacency, max_partition_size, vertex_positions);

		if (best_group == -1)
			continue;

		unsigned int shared = count_shared(groups, top.group_id, best_group, adjacency);

		unsigned int tail = top.group_id;
		while (groups[tail].next >= 0)
			tail = groups[tail].next;

		groups[tail].next = best_group;

		groups[top.group_id].size += groups[best_group].size;
		groups[top.group_id].vertices += groups[best_group].vertices;
		groups[top.group_id].vertices = (groups[top.group_id].vertices > shared) ? groups[top.group_id].vertices - shared : 1;

		groups[best_group].size = 0;
		groups[best_group].vertices = 0;

		if (vertex_positions) {
			merge_bounds(groups[top.group_id], groups[best_group]);
			groups[best_group].radius = 0;
		}

		for (int i = top.group_id; i >= 0; i = groups[i].next)
			groups[i].group = int(top.group_id);

		top.priority = groups[top.group_id].vertices;
		heap_push(order, pending++, top);
	}

	if (vertex_positions) {
		unsigned int* merge_order = reinterpret_cast<unsigned int*>(order);
		size_t merge_offset = 0;

		for (size_t i = 0; i < cluster_count; ++i)
			if (groups[i].size)
				merge_order[merge_offset++] = unsigned(i);

		merge_spatial(groups, merge_order, merge_offset, target_partition_size, max_partition_size, 8, 0);
	}

	size_t next_group = 0;

	for (size_t i = 0; i < cluster_count; ++i) {
		if (groups[i].size == 0)
			continue;

		for (int j = int(i); j >= 0; j = groups[j].next)
			destination[j] = unsigned(next_group);

		next_group++;
	}

	assert(next_group <= cluster_count);
	return next_group;
}

} // namespace mars::mesh
