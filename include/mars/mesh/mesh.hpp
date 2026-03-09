#pragma once

#include <mars/allocator/array_stack_heap_allocator.hpp>

namespace mars::mesh {

enum {
	mesh_simplify_lock_border_flag = 1 << 0,
	mesh_simplify_sparse_flag = 1 << 1,
	mesh_simplify_error_absolute_flag = 1 << 2,
	mesh_simplify_prune_flag = 1 << 3,
	mesh_simplify_regularize_flag = 1 << 4,
	mesh_simplify_permissive_flag = 1 << 5,
};

enum {
	mesh_simplify_vertex_lock = 1 << 0,
	mesh_simplify_vertex_protect = 1 << 1,
};

struct mesh_meshlet {
	unsigned int vertex_offset;
	unsigned int triangle_offset;
	unsigned int vertex_count;
	unsigned int triangle_count;
};

struct mesh_bounds {
	float center[3];
	float radius;
	float cone_apex[3];
	float cone_axis[3];
	float cone_cutoff;
	signed char cone_axis_s8[3];
	signed char cone_cutoff_s8;
};

size_t mesh_build_meshlets(mesh_meshlet* meshlets, unsigned int* meshlet_vertices, unsigned char* meshlet_triangles, const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, size_t max_vertices, size_t max_triangles, float cone_weight);
size_t mesh_build_meshlets_bound(size_t index_count, size_t max_vertices, size_t max_triangles);

size_t mesh_partition_clusters(unsigned int* destination, const unsigned int* cluster_indices, size_t total_index_count, const unsigned int* cluster_index_counts, size_t cluster_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, size_t target_partition_size);

size_t mesh_simplify_with_update(unsigned int* indices, size_t index_count, float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, float* vertex_attributes, size_t vertex_attributes_stride, const float* attribute_weights, size_t attribute_count, const unsigned char* vertex_lock, size_t target_index_count, float target_error, unsigned int options, float* result_error);

inline int mesh_quantize_unorm(float v, int n) {
	const float scale = float((1 << n) - 1);

	v = (v >= 0) ? v : 0;
	v = (v <= 1) ? v : 1;

	return int(v * scale + 0.5f);
}

inline int mesh_quantize_snorm(float v, int n) {
	const float scale = float((1 << (n - 1)) - 1);
	const float round = (v >= 0 ? 0.5f : -0.5f);

	v = (v >= -1) ? v : -1;
	v = (v <= +1) ? v : +1;

	return int(v * scale + round);
}

} // namespace mars::mesh
