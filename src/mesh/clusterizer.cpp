#include <mars/mesh/detail/clusterizer_types.hpp>
#include <mars/mesh/mesh.hpp>

#include <assert.h>
#include <float.h>
#include <math.h>
#include <string.h>

#ifndef MESHOPTIMIZER_NO_SIMD
#if defined(__SSE2__) || (defined(_MSC_VER) && defined(_M_X64))
#define SIMD_SSE
#include <emmintrin.h>
#elif defined(__aarch64__) || (defined(_MSC_VER) && defined(_M_ARM64) && _MSC_VER >= 1922)
#define SIMD_NEON
#include <arm_neon.h>
#endif
#endif

namespace mars::mesh {

namespace core {

const size_t k_meshlet_max_vertices = 256;

const size_t k_meshlet_max_triangles = 512;

const size_t k_meshlet_max_seeds = 256;
const size_t k_meshlet_add_seeds = 4;

const int k_meshlet_max_tree_depth = 50;

static void build_triangle_adjacency(triangle_adjacency& adjacency, const unsigned int* indices, size_t index_count, size_t vertex_count, array_stack_heap_allocator<24>&) {
	size_t face_count = index_count / 3;

	adjacency.counts.assign(vertex_count, 0u);
	adjacency.offsets.resize(vertex_count);
	adjacency.triangles.resize(index_count);

	for (size_t i = 0; i < index_count; ++i) {
		assert(indices[i] < vertex_count);

		adjacency.counts[indices[i]]++;
	}

	unsigned int offset = 0;

	for (size_t i = 0; i < vertex_count; ++i) {
		adjacency.offsets[i] = offset;
		offset += adjacency.counts[i];
	}

	assert(offset == index_count);

	for (size_t i = 0; i < face_count; ++i) {
		unsigned int vertex_a = indices[i * 3 + 0];
		unsigned int vertex_b = indices[i * 3 + 1];
		unsigned int vertex_c = indices[i * 3 + 2];

		adjacency.triangles[adjacency.offsets[vertex_a]++] = unsigned(i);
		adjacency.triangles[adjacency.offsets[vertex_b]++] = unsigned(i);
		adjacency.triangles[adjacency.offsets[vertex_c]++] = unsigned(i);
	}

	for (size_t i = 0; i < vertex_count; ++i) {
		assert(adjacency.offsets[i] >= adjacency.counts[i]);
		adjacency.offsets[i] -= adjacency.counts[i];
	}
}

static void build_triangle_adjacency_sparse(triangle_adjacency& adjacency, const unsigned int* indices, size_t index_count, size_t vertex_count, array_stack_heap_allocator<24>&) {
	size_t face_count = index_count / 3;

	const unsigned int sparse_seen = 1u << 31;
	assert(index_count < sparse_seen);

	adjacency.counts.resize(vertex_count);
	adjacency.offsets.resize(vertex_count);
	adjacency.triangles.resize(index_count);

	for (size_t i = 0; i < index_count; ++i)
		assert(indices[i] < vertex_count);

	for (size_t i = 0; i < index_count; ++i)
		adjacency.counts[indices[i]] = 0;

	for (size_t i = 0; i < index_count; ++i)
		adjacency.counts[indices[i]]++;

	unsigned int offset = 0;

	for (size_t i = 0; i < index_count; ++i) {
		unsigned int vertex_index = indices[i];

		if ((adjacency.counts[vertex_index] & sparse_seen) == 0) {
			adjacency.offsets[vertex_index] = offset;
			offset += adjacency.counts[vertex_index];
			adjacency.counts[vertex_index] |= sparse_seen;
		}
	}

	assert(offset == index_count);

	for (size_t i = 0; i < face_count; ++i) {
		unsigned int vertex_a = indices[i * 3 + 0];
		unsigned int vertex_b = indices[i * 3 + 1];
		unsigned int vertex_c = indices[i * 3 + 2];

		adjacency.triangles[adjacency.offsets[vertex_a]++] = unsigned(i);
		adjacency.triangles[adjacency.offsets[vertex_b]++] = unsigned(i);
		adjacency.triangles[adjacency.offsets[vertex_c]++] = unsigned(i);
	}

	for (size_t i = 0; i < index_count; ++i) {
		unsigned int vertex_index = indices[i];

		if (adjacency.counts[vertex_index] & sparse_seen) {
			adjacency.counts[vertex_index] &= ~sparse_seen;

			assert(adjacency.offsets[vertex_index] >= adjacency.counts[vertex_index]);
			adjacency.offsets[vertex_index] -= adjacency.counts[vertex_index];
		}
	}
}

static void clear_used(short* used, size_t vertex_count, const unsigned int* indices, size_t index_count) {
	if (vertex_count <= index_count)
		memset(used, -1, vertex_count * sizeof(short));
	else
		for (size_t i = 0; i < index_count; ++i) {
			assert(indices[i] < vertex_count);
			used[indices[i]] = -1;
		}
}

static void compute_bounding_sphere(float result[4], const float* points, size_t count, size_t points_stride, const float* radii, size_t radii_stride, size_t axis_count) {
	static const float k_axes[7][3] = {

		{1, 0, 0},
		{0, 1, 0},
		{0, 0, 1},

		{0.57735026f, 0.57735026f, 0.57735026f},
		{-0.57735026f, 0.57735026f, 0.57735026f},
		{0.57735026f, -0.57735026f, 0.57735026f},
		{0.57735026f, 0.57735026f, -0.57735026f},
	};

	assert(count > 0);
	assert(axis_count <= sizeof(k_axes) / sizeof(k_axes[0]));

	size_t points_stride_float = points_stride / sizeof(float);
	size_t radii_stride_float = radii_stride / sizeof(float);

	size_t min_point_indices[7], max_point_indices[7];
	float min_projections[7], max_projections[7];

	for (size_t axis = 0; axis < axis_count; ++axis) {
		min_point_indices[axis] = max_point_indices[axis] = 0;
		min_projections[axis] = FLT_MAX;
		max_projections[axis] = -FLT_MAX;
	}

	for (size_t i = 0; i < count; ++i) {
		const float* point = points + i * points_stride_float;
		float point_radius = radii[i * radii_stride_float];

		for (size_t axis = 0; axis < axis_count; ++axis) {
			const float* axis_vector = k_axes[axis];

			float projected_distance = axis_vector[0] * point[0] + axis_vector[1] * point[1] + axis_vector[2] * point[2];
			float projected_min = projected_distance - point_radius;
			float projected_max = projected_distance + point_radius;

			min_point_indices[axis] = (projected_min < min_projections[axis]) ? i : min_point_indices[axis];
			max_point_indices[axis] = (projected_max > max_projections[axis]) ? i : max_point_indices[axis];
			min_projections[axis] = (projected_min < min_projections[axis]) ? projected_min : min_projections[axis];
			max_projections[axis] = (projected_max > max_projections[axis]) ? projected_max : max_projections[axis];
		}
	}

	size_t longest_axis = 0;
	float longest_diameter = 0;

	for (size_t axis = 0; axis < axis_count; ++axis) {
		const float* min_point = points + min_point_indices[axis] * points_stride_float;
		const float* max_point = points + max_point_indices[axis] * points_stride_float;
		float min_radius = radii[min_point_indices[axis] * radii_stride_float];
		float max_radius = radii[max_point_indices[axis] * radii_stride_float];

		float distance_sq = (max_point[0] - min_point[0]) * (max_point[0] - min_point[0]) + (max_point[1] - min_point[1]) * (max_point[1] - min_point[1]) + (max_point[2] - min_point[2]) * (max_point[2] - min_point[2]);
		float diameter = sqrtf(distance_sq) + min_radius + max_radius;

		if (diameter > longest_diameter) {
			longest_diameter = diameter;
			longest_axis = axis;
		}
	}

	const float* min_point = points + min_point_indices[longest_axis] * points_stride_float;
	const float* max_point = points + max_point_indices[longest_axis] * points_stride_float;
	float min_radius = radii[min_point_indices[longest_axis] * radii_stride_float];
	float max_radius = radii[max_point_indices[longest_axis] * radii_stride_float];

	float axis_distance = sqrtf((max_point[0] - min_point[0]) * (max_point[0] - min_point[0]) + (max_point[1] - min_point[1]) * (max_point[1] - min_point[1]) + (max_point[2] - min_point[2]) * (max_point[2] - min_point[2]));
	float axis_lerp = axis_distance > 0 ? (axis_distance + max_radius - min_radius) / (2 * axis_distance) : 0.f;

	float center[3] = {min_point[0] + (max_point[0] - min_point[0]) * axis_lerp, min_point[1] + (max_point[1] - min_point[1]) * axis_lerp, min_point[2] + (max_point[2] - min_point[2]) * axis_lerp};
	float radius = longest_diameter / 2;

	for (size_t i = 0; i < count; ++i) {
		const float* point = points + i * points_stride_float;
		float point_radius = radii[i * radii_stride_float];

		float distance_sq = (point[0] - center[0]) * (point[0] - center[0]) + (point[1] - center[1]) * (point[1] - center[1]) + (point[2] - center[2]) * (point[2] - center[2]);
		float distance = sqrtf(distance_sq);

		if (distance + point_radius > radius) {
			float expansion = distance > 0 ? (distance + point_radius - radius) / (2 * distance) : 0.f;

			center[0] += expansion * (point[0] - center[0]);
			center[1] += expansion * (point[1] - center[1]);
			center[2] += expansion * (point[2] - center[2]);
			radius = (radius + distance + point_radius) / 2;
		}
	}

	result[0] = center[0];
	result[1] = center[1];
	result[2] = center[2];
	result[3] = radius;
}

static float get_meshlet_score(float distance, float spread, float cone_weight, float expected_radius) {
	float cone = 1.f - spread * cone_weight;
	float cone_clamped = cone < 1e-3f ? 1e-3f : cone;

	return (1 + distance / expected_radius * (1 - cone_weight)) * cone_clamped;
}

static surface_cone get_meshlet_cone(const surface_cone& accumulated_cone, unsigned int triangle_count) {
	surface_cone result = accumulated_cone;

	float center_scale = triangle_count == 0 ? 0.f : 1.f / float(triangle_count);

	result.center.x *= center_scale;
	result.center.y *= center_scale;
	result.center.z *= center_scale;

	float axis_length = result.axis.x * result.axis.x + result.axis.y * result.axis.y + result.axis.z * result.axis.z;
	float axis_scale = axis_length == 0.f ? 0.f : 1.f / sqrtf(axis_length);

	result.axis.x *= axis_scale;
	result.axis.y *= axis_scale;
	result.axis.z *= axis_scale;

	return result;
}

static float compute_triangle_cones(surface_cone* triangle_cones, const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride) {
	(void)vertex_count;

	size_t vertex_stride_float = vertex_positions_stride / sizeof(float);
	size_t face_count = index_count / 3;

	float mesh_area = 0;

	for (size_t i = 0; i < face_count; ++i) {
		unsigned int vertex_a = indices[i * 3 + 0];
		unsigned int vertex_b = indices[i * 3 + 1];
		unsigned int vertex_c = indices[i * 3 + 2];
		assert(vertex_a < vertex_count && vertex_b < vertex_count && vertex_c < vertex_count);

		const float* position_a = vertex_positions + vertex_stride_float * vertex_a;
		const float* position_b = vertex_positions + vertex_stride_float * vertex_b;
		const float* position_c = vertex_positions + vertex_stride_float * vertex_c;

		float edge_ab[3] = {position_b[0] - position_a[0], position_b[1] - position_a[1], position_b[2] - position_a[2]};
		float edge_ac[3] = {position_c[0] - position_a[0], position_c[1] - position_a[1], position_c[2] - position_a[2]};

		float normal_x = edge_ab[1] * edge_ac[2] - edge_ab[2] * edge_ac[1];
		float normal_y = edge_ab[2] * edge_ac[0] - edge_ab[0] * edge_ac[2];
		float normal_z = edge_ab[0] * edge_ac[1] - edge_ab[1] * edge_ac[0];

		float area = sqrtf(normal_x * normal_x + normal_y * normal_y + normal_z * normal_z);
		float inverse_area = (area == 0.f) ? 0.f : 1.f / area;

		triangle_cones[i].center.x = (position_a[0] + position_b[0] + position_c[0]) / 3.f;
		triangle_cones[i].center.y = (position_a[1] + position_b[1] + position_c[1]) / 3.f;
		triangle_cones[i].center.z = (position_a[2] + position_b[2] + position_c[2]) / 3.f;

		triangle_cones[i].axis.x = normal_x * inverse_area;
		triangle_cones[i].axis.y = normal_y * inverse_area;
		triangle_cones[i].axis.z = normal_z * inverse_area;

		mesh_area += area;
	}

	return mesh_area;
}

static bool append_meshlet(mesh_meshlet& meshlet, unsigned int a, unsigned int b, unsigned int c, short* used, mesh_meshlet* meshlets, unsigned int* meshlet_vertices, unsigned char* meshlet_triangles, size_t meshlet_offset, size_t max_vertices, size_t max_triangles, bool split = false) {
	short& local_vertex_a = used[a];
	short& local_vertex_b = used[b];
	short& local_vertex_c = used[c];

	bool flushed_meshlet = false;

	int new_vertex_count = (local_vertex_a < 0) + (local_vertex_b < 0) + (local_vertex_c < 0);

	if (meshlet.vertex_count + new_vertex_count > max_vertices || meshlet.triangle_count >= max_triangles || split) {
		meshlets[meshlet_offset] = meshlet;

		for (size_t j = 0; j < meshlet.vertex_count; ++j)
			used[meshlet_vertices[meshlet.vertex_offset + j]] = -1;

		meshlet.vertex_offset += meshlet.vertex_count;
		meshlet.triangle_offset += meshlet.triangle_count * 3;
		meshlet.vertex_count = 0;
		meshlet.triangle_count = 0;

		flushed_meshlet = true;
	}

	if (local_vertex_a < 0) {
		local_vertex_a = short(meshlet.vertex_count);
		meshlet_vertices[meshlet.vertex_offset + meshlet.vertex_count++] = a;
	}

	if (local_vertex_b < 0) {
		local_vertex_b = short(meshlet.vertex_count);
		meshlet_vertices[meshlet.vertex_offset + meshlet.vertex_count++] = b;
	}

	if (local_vertex_c < 0) {
		local_vertex_c = short(meshlet.vertex_count);
		meshlet_vertices[meshlet.vertex_offset + meshlet.vertex_count++] = c;
	}

	meshlet_triangles[meshlet.triangle_offset + meshlet.triangle_count * 3 + 0] = (unsigned char)local_vertex_a;
	meshlet_triangles[meshlet.triangle_offset + meshlet.triangle_count * 3 + 1] = (unsigned char)local_vertex_b;
	meshlet_triangles[meshlet.triangle_offset + meshlet.triangle_count * 3 + 2] = (unsigned char)local_vertex_c;
	meshlet.triangle_count++;

	return flushed_meshlet;
}

static unsigned int get_neighbor_triangle(const mesh_meshlet& meshlet, const surface_cone& meshlet_cone, const unsigned int* meshlet_vertices, const unsigned int* indices, const triangle_adjacency& adjacency, const surface_cone* triangle_cones, const unsigned int* live_triangles, const short* used, float meshlet_expected_radius, float cone_weight) {
	unsigned int best_triangle = ~0u;
	int best_priority = 5;
	float best_score = FLT_MAX;

	for (size_t i = 0; i < meshlet.vertex_count; ++i) {
		unsigned int index = meshlet_vertices[meshlet.vertex_offset + i];

		const unsigned int* neighbors = adjacency.triangles.data() + adjacency.offsets[index];
		size_t neighbors_size = adjacency.counts[index];

		for (size_t j = 0; j < neighbors_size; ++j) {
			unsigned int triangle = neighbors[j];
			unsigned int vertex_a = indices[triangle * 3 + 0];
			unsigned int vertex_b = indices[triangle * 3 + 1];
			unsigned int vertex_c = indices[triangle * 3 + 2];

			int additional_vertices = (used[vertex_a] < 0) + (used[vertex_b] < 0) + (used[vertex_c] < 0);
			assert(additional_vertices <= 2);

			int priority = -1;

			if (additional_vertices == 0)
				priority = 0;

			else if (live_triangles[vertex_a] == 1 || live_triangles[vertex_b] == 1 || live_triangles[vertex_c] == 1)
				priority = 1;

			else if ((live_triangles[vertex_a] == 2) + (live_triangles[vertex_b] == 2) + (live_triangles[vertex_c] == 2) >= 2)
				priority = 1 + additional_vertices;

			else
				priority = 2 + additional_vertices;

			if (priority > best_priority)
				continue;

			const surface_cone& triangle_cone = triangle_cones[triangle];

			float delta_x = triangle_cone.center.x - meshlet_cone.center.x;
			float delta_y = triangle_cone.center.y - meshlet_cone.center.y;
			float delta_z = triangle_cone.center.z - meshlet_cone.center.z;
			float distance = sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
			float spread = triangle_cone.axis.x * meshlet_cone.axis.x + triangle_cone.axis.y * meshlet_cone.axis.y + triangle_cone.axis.z * meshlet_cone.axis.z;

			float score = get_meshlet_score(distance, spread, cone_weight, meshlet_expected_radius);

			if (priority < best_priority || score < best_score) {
				best_triangle = triangle;
				best_priority = priority;
				best_score = score;
			}
		}
	}

	return best_triangle;
}

static size_t append_seed_triangles(unsigned int* seeds, const mesh_meshlet& meshlet, const unsigned int* meshlet_vertices, const unsigned int* indices, const triangle_adjacency& adjacency, const surface_cone* triangle_cones, const unsigned int* live_triangles, float scene_min_x, float scene_min_y, float scene_min_z) {
	unsigned int best_seeds[k_meshlet_add_seeds];
	unsigned int best_live_triangle_counts[k_meshlet_add_seeds];
	float best_seed_scores[k_meshlet_add_seeds];

	for (size_t i = 0; i < k_meshlet_add_seeds; ++i) {
		best_seeds[i] = ~0u;
		best_live_triangle_counts[i] = ~0u;
		best_seed_scores[i] = FLT_MAX;
	}

	for (size_t i = 0; i < meshlet.vertex_count; ++i) {
		unsigned int index = meshlet_vertices[meshlet.vertex_offset + i];

		unsigned int best_neighbor_triangle = ~0u;
		unsigned int best_neighbor_live_triangle_count = ~0u;

		const unsigned int* neighbors = adjacency.triangles.data() + adjacency.offsets[index];
		size_t neighbors_size = adjacency.counts[index];

		for (size_t j = 0; j < neighbors_size; ++j) {
			unsigned int triangle = neighbors[j];
			unsigned int vertex_a = indices[triangle * 3 + 0];
			unsigned int vertex_b = indices[triangle * 3 + 1];
			unsigned int vertex_c = indices[triangle * 3 + 2];

			unsigned int live_triangle_count = live_triangles[vertex_a] + live_triangles[vertex_b] + live_triangles[vertex_c];

			if (live_triangle_count < best_neighbor_live_triangle_count) {
				best_neighbor_triangle = triangle;
				best_neighbor_live_triangle_count = live_triangle_count;
			}
		}

		if (best_neighbor_triangle == ~0u)
			continue;

		float delta_x = triangle_cones[best_neighbor_triangle].center.x - scene_min_x;
		float delta_y = triangle_cones[best_neighbor_triangle].center.y - scene_min_y;
		float delta_z = triangle_cones[best_neighbor_triangle].center.z - scene_min_z;
		float best_neighbor_score = sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);

		for (size_t j = 0; j < k_meshlet_add_seeds; ++j) {
			if (best_neighbor_live_triangle_count < best_live_triangle_counts[j] || (best_neighbor_live_triangle_count == best_live_triangle_counts[j] && best_neighbor_score <= best_seed_scores[j])) {
				best_seeds[j] = best_neighbor_triangle;
				best_live_triangle_counts[j] = best_neighbor_live_triangle_count;
				best_seed_scores[j] = best_neighbor_score;
				break;
			}
		}
	}

	size_t seed_count = 0;

	for (size_t i = 0; i < k_meshlet_add_seeds; ++i)
		if (best_seeds[i] != ~0u)
			seeds[seed_count++] = best_seeds[i];

	return seed_count;
}

static size_t prune_seed_triangles(unsigned int* seeds, size_t seed_count, const unsigned char* emitted_flags) {
	size_t result = 0;

	for (size_t i = 0; i < seed_count; ++i) {
		unsigned int index = seeds[i];

		seeds[result] = index;
		result += emitted_flags[index] == 0;
	}

	return result;
}

static unsigned int select_seed_triangle(const unsigned int* seeds, size_t seed_count, const unsigned int* indices, const surface_cone* triangle_cones, const unsigned int* live_triangles, float scene_min_x, float scene_min_y, float scene_min_z) {
	unsigned int best_seed = ~0u;
	unsigned int best_live_triangle_count = ~0u;
	float best_score = FLT_MAX;

	for (size_t i = 0; i < seed_count; ++i) {
		unsigned int index = seeds[i];
		unsigned int vertex_a = indices[index * 3 + 0];
		unsigned int vertex_b = indices[index * 3 + 1];
		unsigned int vertex_c = indices[index * 3 + 2];

		unsigned int live_triangle_count = live_triangles[vertex_a] + live_triangles[vertex_b] + live_triangles[vertex_c];
		float delta_x = triangle_cones[index].center.x - scene_min_x;
		float delta_y = triangle_cones[index].center.y - scene_min_y;
		float delta_z = triangle_cones[index].center.z - scene_min_z;
		float score = sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);

		if (live_triangle_count < best_live_triangle_count || (live_triangle_count == best_live_triangle_count && score < best_score)) {
			best_seed = index;
			best_live_triangle_count = live_triangle_count;
			best_score = score;
		}
	}

	return best_seed;
}

static size_t kdtree_partition(unsigned int* indices, size_t count, const float* points, size_t stride, int axis, float pivot) {
	size_t write_offset = 0;

	for (size_t i = 0; i < count; ++i) {
		float coordinate = points[indices[i] * stride + axis];

		unsigned int swapped_index = indices[write_offset];
		indices[write_offset] = indices[i];
		indices[i] = swapped_index;

		write_offset += coordinate < pivot;
	}

	return write_offset;
}

static size_t kdtree_build_leaf(size_t offset, kd_tree_node* nodes, size_t node_count, unsigned int* indices, size_t count) {
	assert(offset + count <= node_count);
	(void)node_count;

	kd_tree_node& result = nodes[offset];

	result.primitive_index = indices[0];
	result.split_axis = 3;
	result.child_offset = unsigned(count);

	for (size_t i = 1; i < count; ++i) {
		kd_tree_node& tail = nodes[offset + i];

		tail.primitive_index = indices[i];
		tail.split_axis = 3;
		tail.child_offset = ~0u >> 2;
	}

	return offset + count;
}

static size_t kdtree_build(size_t offset, kd_tree_node* nodes, size_t node_count, const float* points, size_t stride, unsigned int* indices, size_t count, size_t leaf_size, int depth) {
	assert(count > 0);
	assert(offset < node_count);

	if (count <= leaf_size)
		return kdtree_build_leaf(offset, nodes, node_count, indices, count);

	float mean[3] = {};
	float variance[3] = {};
	float running_count = 1;
	float inverse_running_count = 1;

	for (size_t i = 0; i < count; ++i, running_count += 1.f, inverse_running_count = 1.f / running_count) {
		const float* point = points + indices[i] * stride;

		for (int k = 0; k < 3; ++k) {
			float delta = point[k] - mean[k];
			mean[k] += delta * inverse_running_count;
			variance[k] += delta * (point[k] - mean[k]);
		}
	}

	int split_axis = (variance[0] >= variance[1] && variance[0] >= variance[2]) ? 0 : (variance[1] >= variance[2] ? 1 : 2);

	float split_position = mean[split_axis];
	size_t middle = kdtree_partition(indices, count, points, stride, split_axis, split_position);

	if (middle <= leaf_size / 2 || middle >= count - leaf_size / 2 || depth >= k_meshlet_max_tree_depth)
		return kdtree_build_leaf(offset, nodes, node_count, indices, count);

	kd_tree_node& result = nodes[offset];

	result.split_position = split_position;
	result.split_axis = split_axis;

	size_t next_offset = kdtree_build(offset + 1, nodes, node_count, points, stride, indices, middle, leaf_size, depth + 1);

	assert(next_offset - offset > 1);
	result.child_offset = unsigned(next_offset - offset - 1);

	return kdtree_build(next_offset, nodes, node_count, points, stride, indices + middle, count - middle, leaf_size, depth + 1);
}

static void kdtree_nearest(kd_tree_node* nodes, unsigned int node_index, const float* points, size_t stride, const unsigned char* emitted_flags, const float* query_position, unsigned int& result, float& distance_limit) {
	const kd_tree_node& node = nodes[node_index];

	if (node.child_offset == 0)
		return;

	if (node.split_axis == 3) {
		bool inactive = true;

		for (unsigned int i = 0; i < node.child_offset; ++i) {
			unsigned int primitive_index = nodes[node_index + i].primitive_index;

			if (emitted_flags[primitive_index])
				continue;

			inactive = false;

			const float* point = points + primitive_index * stride;

			float delta_x = point[0] - query_position[0];
			float delta_y = point[1] - query_position[1];
			float delta_z = point[2] - query_position[2];
			float distance = sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);

			if (distance < distance_limit) {
				result = primitive_index;
				distance_limit = distance;
			}
		}

		if (inactive)
			nodes[node_index].child_offset = 0;
	} else {
		float split_delta = query_position[node.split_axis] - node.split_position;
		unsigned int first_child_offset = (split_delta <= 0) ? 0 : node.child_offset;
		unsigned int second_child_offset = first_child_offset ^ node.child_offset;

		if ((nodes[node_index + 1 + first_child_offset].child_offset | nodes[node_index + 1 + second_child_offset].child_offset) == 0)
			nodes[node_index].child_offset = 0;

		kdtree_nearest(nodes, node_index + 1 + first_child_offset, points, stride, emitted_flags, query_position, result, distance_limit);

		if (fabsf(split_delta) <= distance_limit)
			kdtree_nearest(nodes, node_index + 1 + second_child_offset, points, stride, emitted_flags, query_position, result, distance_limit);
	}
}

#if defined(SIMD_SSE)
static float box_merge(simd_bvh_bounds& bounds, const bvh_bounds& other_bounds) {
	__m128 min = _mm_loadu_ps(&bounds.min_corner.x);
	__m128 max = _mm_loadu_ps(&bounds.max_corner.x);

	min = _mm_min_ps(min, _mm_loadu_ps(&other_bounds.min_corner.x));
	max = _mm_max_ps(max, _mm_loadu_ps(&other_bounds.max_corner.x));

	_mm_storeu_ps(&bounds.min_corner.x, min);
	_mm_storeu_ps(&bounds.max_corner.x, max);

	__m128 size = _mm_sub_ps(max, min);
	__m128 size_yzx = _mm_shuffle_ps(size, size, _MM_SHUFFLE(0, 0, 2, 1));
	__m128 mul = _mm_mul_ps(size, size_yzx);
	__m128 sum_xy = _mm_add_ss(mul, _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(1, 1, 1, 1)));
	__m128 sum_xyz = _mm_add_ss(sum_xy, _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 2, 2, 2)));

	return _mm_cvtss_f32(sum_xyz);
}
#elif defined(SIMD_NEON)
static float box_merge(simd_bvh_bounds& bounds, const bvh_bounds& other_bounds) {
	float32x4_t min = vld1q_f32(&bounds.min_corner.x);
	float32x4_t max = vld1q_f32(&bounds.max_corner.x);

	min = vminq_f32(min, vld1q_f32(&other_bounds.min_corner.x));
	max = vmaxq_f32(max, vld1q_f32(&other_bounds.max_corner.x));

	vst1q_f32(&bounds.min_corner.x, min);
	vst1q_f32(&bounds.max_corner.x, max);

	float32x4_t size = vsubq_f32(max, min);
	float32x4_t size_yzx = vextq_f32(vextq_f32(size, size, 3), size, 2);
	float32x4_t mul = vmulq_f32(size, size_yzx);
	float sum_xy = vgetq_lane_f32(mul, 0) + vgetq_lane_f32(mul, 1);
	float sum_xyz = sum_xy + vgetq_lane_f32(mul, 2);

	return sum_xyz;
}
#else
static float box_merge(simd_bvh_bounds& bounds, const bvh_bounds& other_bounds) {
	for (int k = 0; k < 3; ++k) {
		bounds.min_corner[k] = other_bounds.min_corner[k] < bounds.min_corner[k] ? other_bounds.min_corner[k] : bounds.min_corner[k];
		bounds.max_corner[k] = other_bounds.max_corner[k] > bounds.max_corner[k] ? other_bounds.max_corner[k] : bounds.max_corner[k];
	}

	float sx = bounds.max_corner.x - bounds.min_corner.x, sy = bounds.max_corner.y - bounds.min_corner.y, sz = bounds.max_corner.z - bounds.min_corner.z;
	return sx * sy + sx * sz + sy * sz;
}
#endif

inline unsigned int radix_float(unsigned int v) {
	unsigned int mask = (int(v) >> 31) | 0x80000000;
	return v ^ mask;
}

static void compute_histogram(unsigned int (&hist)[1024][3], const float* data, size_t count) {
	memset(hist, 0, sizeof(hist));

	const unsigned int* bits = reinterpret_cast<const unsigned int*>(data);

	for (size_t i = 0; i < count; ++i) {
		unsigned int id = radix_float(bits[i]);

		hist[(id >> 2) & 1023][0]++;
		hist[(id >> 12) & 1023][1]++;
		hist[(id >> 22) & 1023][2]++;
	}

	unsigned int sum0 = 0, sum1 = 0, sum2 = 0;

	for (int i = 0; i < 1024; ++i) {
		unsigned int bucket_count_x = hist[i][0];
		unsigned int bucket_count_y = hist[i][1];
		unsigned int bucket_count_z = hist[i][2];

		hist[i][0] = sum0;
		hist[i][1] = sum1;
		hist[i][2] = sum2;

		sum0 += bucket_count_x;
		sum1 += bucket_count_y;
		sum2 += bucket_count_z;
	}

	assert(sum0 == count && sum1 == count && sum2 == count);
}

static void radix_pass(unsigned int* destination, const unsigned int* source, const float* keys, size_t count, unsigned int (&hist)[1024][3], int pass) {
	const unsigned int* bits = reinterpret_cast<const unsigned int*>(keys);
	int bitoff = pass * 10 + 2;

	for (size_t i = 0; i < count; ++i) {
		unsigned int id = (radix_float(bits[source[i]]) >> bitoff) & 1023;

		destination[hist[id][pass]++] = source[i];
	}
}

static void bvh_prepare(bvh_bounds* triangle_bounds, float* centroids, const unsigned int* indices, size_t face_count, const float* vertex_positions, size_t vertex_count, size_t vertex_stride_float) {
	(void)vertex_count;

	for (size_t i = 0; i < face_count; ++i) {
		unsigned int vertex_a = indices[i * 3 + 0];
		unsigned int vertex_b = indices[i * 3 + 1];
		unsigned int vertex_c = indices[i * 3 + 2];
		assert(vertex_a < vertex_count && vertex_b < vertex_count && vertex_c < vertex_count);

		const float* position_a = vertex_positions + vertex_stride_float * vertex_a;
		const float* position_b = vertex_positions + vertex_stride_float * vertex_b;
		const float* position_c = vertex_positions + vertex_stride_float * vertex_c;

		bvh_bounds& bounds = triangle_bounds[i];

		for (int k = 0; k < 3; ++k) {
			bounds.min_corner[k] = position_a[k] < position_b[k] ? position_a[k] : position_b[k];
			bounds.min_corner[k] = position_c[k] < bounds.min_corner[k] ? position_c[k] : bounds.min_corner[k];

			bounds.max_corner[k] = position_a[k] > position_b[k] ? position_a[k] : position_b[k];
			bounds.max_corner[k] = position_c[k] > bounds.max_corner[k] ? position_c[k] : bounds.max_corner[k];

			centroids[i + face_count * k] = (bounds.min_corner[k] + bounds.max_corner[k]) / 2.f;
		}
	}
}

static size_t bvh_count_vertices(const unsigned int* order, size_t count, short* used, const unsigned int* indices, unsigned int* out = NULL) {
	size_t used_vertices = 0;
	for (size_t i = 0; i < count; ++i) {
		unsigned int index = order[i];
		unsigned int a = indices[index * 3 + 0], b = indices[index * 3 + 1], c = indices[index * 3 + 2];

		used_vertices += (used[a] < 0) + (used[b] < 0) + (used[c] < 0);
		used[a] = used[b] = used[c] = 1;

		if (out)
			out[i] = unsigned(used_vertices);
	}

	for (size_t i = 0; i < count; ++i) {
		unsigned int index = order[i];
		unsigned int a = indices[index * 3 + 0], b = indices[index * 3 + 1], c = indices[index * 3 + 2];

		used[a] = used[b] = used[c] = -1;
	}

	return used_vertices;
}

static void bvh_pack_leaf(unsigned char* boundary, size_t count) {
	assert(count > 0);

	boundary[0] = 1;
	memset(boundary + 1, 0, count - 1);
}

static void bvh_pack_tail(unsigned char* boundary, const unsigned int* order, size_t count, short* used, const unsigned int* indices, size_t max_vertices, size_t max_triangles) {
	for (size_t i = 0; i < count;) {
		size_t chunk = i + max_triangles <= count ? max_triangles : count - i;

		if (bvh_count_vertices(order + i, chunk, used, indices) <= max_vertices) {
			bvh_pack_leaf(boundary + i, chunk);
			i += chunk;
			continue;
		}

		assert(chunk > max_vertices / 3);

		bvh_pack_leaf(boundary + i, max_vertices / 3);
		i += max_vertices / 3;
	}
}

static bool bvh_divisible(size_t count, size_t min, size_t max) {
	return min * 2 <= max ? count >= min : count % min <= (count / min) * (max - min);
}

static void bvh_compute_area(float* areas, const bvh_bounds* triangle_bounds, const unsigned int* order, size_t count) {
	simd_bvh_bounds left_accum = {{FLT_MAX, FLT_MAX, FLT_MAX, 0}, {-FLT_MAX, -FLT_MAX, -FLT_MAX, 0}};
	simd_bvh_bounds right_accum = left_accum;

	for (size_t i = 0; i < count; ++i) {
		float larea = box_merge(left_accum, triangle_bounds[order[i]]);
		float rarea = box_merge(right_accum, triangle_bounds[order[count - 1 - i]]);

		areas[i] = larea;
		areas[i + count] = rarea;
	}
}

static size_t bvh_pivot(const float* areas, const unsigned int* vertices, size_t count, size_t step, size_t min, size_t max, float fill, size_t maxfill, float* out_cost) {
	bool aligned = count >= min * 2 && bvh_divisible(count, min, max);
	size_t end = aligned ? count - min : count - 1;

	float rmaxfill = 1.f / float(int(maxfill));

	size_t bestsplit = 0;
	float bestcost = FLT_MAX;

	for (size_t i = min - 1; i < end; i += step) {
		size_t lsplit = i + 1, rsplit = count - (i + 1);

		if (!bvh_divisible(lsplit, min, max))
			continue;
		if (aligned && !bvh_divisible(rsplit, min, max))
			continue;

		float larea = areas[i], rarea = areas[(count - 1 - (i + 1)) + count];
		float cost = larea * float(int(lsplit)) + rarea * float(int(rsplit));

		if (cost > bestcost)
			continue;

		size_t lfill = vertices ? vertices[i] : lsplit;
		size_t rfill = vertices ? vertices[i] : rsplit;

		int lrest = int(float(int(lfill + maxfill - 1)) * rmaxfill) * int(maxfill) - int(lfill);
		int rrest = int(float(int(rfill + maxfill - 1)) * rmaxfill) * int(maxfill) - int(rfill);

		cost += fill * (float(lrest) * larea + float(rrest) * rarea);

		if (cost < bestcost) {
			bestcost = cost;
			bestsplit = i + 1;
		}
	}

	*out_cost = bestcost;
	return bestsplit;
}

static void bvh_partition(unsigned int* target, const unsigned int* order, const unsigned char* sides, size_t split, size_t count) {
	size_t l = 0, r = split;

	for (size_t i = 0; i < count; ++i) {
		unsigned char side = sides[order[i]];
		target[side ? r : l] = order[i];
		l += 1;
		l -= side;
		r += side;
	}

	assert(l == split && r == count);
}

static void bvh_split(const bvh_bounds* triangle_bounds, unsigned int* orderx, unsigned int* ordery, unsigned int* orderz, unsigned char* boundary, size_t count, int depth, void* scratch, short* used, const unsigned int* indices, size_t max_vertices, size_t min_triangles, size_t max_triangles, float fill_weight) {
	if (count <= max_triangles && bvh_count_vertices(orderx, count, used, indices) <= max_vertices)
		return bvh_pack_leaf(boundary, count);

	unsigned int* axes[3] = {orderx, ordery, orderz};

	size_t step = min_triangles == max_triangles && count > max_triangles ? max_triangles : 1;

	size_t mint = count <= max_triangles && max_vertices / 3 < min_triangles ? max_vertices / 3 : min_triangles;
	size_t maxfill = count <= max_triangles ? max_vertices : max_triangles;

	int bestk = -1;
	size_t bestsplit = 0;
	float bestcost = FLT_MAX;

	for (int k = 0; k < 3; ++k) {
		float* areas = static_cast<float*>(scratch);
		unsigned int* vertices = NULL;

		bvh_compute_area(areas, triangle_bounds, axes[k], count);

		if (count <= max_triangles) {
			vertices = reinterpret_cast<unsigned int*>(areas + 2 * count);
			bvh_count_vertices(axes[k], count, used, indices, vertices);
		}

		float axiscost = FLT_MAX;
		size_t axissplit = bvh_pivot(areas, vertices, count, step, mint, max_triangles, fill_weight, maxfill, &axiscost);

		if (axissplit && axiscost < bestcost) {
			bestk = k;
			bestcost = axiscost;
			bestsplit = axissplit;
		}
	}

	if (bestk < 0 || depth >= k_meshlet_max_tree_depth)
		return bvh_pack_tail(boundary, orderx, count, used, indices, max_vertices, max_triangles);

	unsigned char* sides = static_cast<unsigned char*>(scratch) + count * sizeof(unsigned int);

	for (size_t i = 0; i < bestsplit; ++i)
		sides[axes[bestk][i]] = 0;

	for (size_t i = bestsplit; i < count; ++i)
		sides[axes[bestk][i]] = 1;

	unsigned int* temp = static_cast<unsigned int*>(scratch);

	for (int k = 0; k < 3; ++k) {
		if (k == bestk)
			continue;

		unsigned int* axis = axes[k];
		memcpy(temp, axis, sizeof(unsigned int) * count);
		bvh_partition(axis, temp, sides, bestsplit, count);
	}

	bvh_split(triangle_bounds, orderx, ordery, orderz, boundary, bestsplit, depth + 1, scratch, used, indices, max_vertices, min_triangles, max_triangles, fill_weight);
	bvh_split(triangle_bounds, orderx + bestsplit, ordery + bestsplit, orderz + bestsplit, boundary + bestsplit, count - bestsplit, depth + 1, scratch, used, indices, max_vertices, min_triangles, max_triangles, fill_weight);
}

} // namespace core

size_t mesh_build_meshlets_bound(size_t index_count, size_t max_vertices, size_t max_triangles) {
	using namespace mars::mesh::core;

	assert(index_count % 3 == 0);
	assert(max_vertices >= 3 && max_vertices <= k_meshlet_max_vertices);
	assert(max_triangles >= 1 && max_triangles <= k_meshlet_max_triangles);

	(void)k_meshlet_max_vertices;
	(void)k_meshlet_max_triangles;

	size_t max_vertices_conservative = max_vertices - 2;
	size_t meshlet_limit_vertices = (index_count + max_vertices_conservative - 1) / max_vertices_conservative;
	size_t meshlet_limit_triangles = (index_count / 3 + max_triangles - 1) / max_triangles;

	return meshlet_limit_vertices > meshlet_limit_triangles ? meshlet_limit_vertices : meshlet_limit_triangles;
}

size_t mesh_build_meshlets_flex(mesh_meshlet* meshlets, unsigned int* meshlet_vertices, unsigned char* meshlet_triangles, const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, size_t max_vertices, size_t min_triangles, size_t max_triangles, float cone_weight, float split_factor) {
	using namespace mars::mesh::core;

	assert(index_count % 3 == 0);
	assert(vertex_positions_stride >= 12 && vertex_positions_stride <= 256);
	assert(vertex_positions_stride % sizeof(float) == 0);

	assert(max_vertices >= 3 && max_vertices <= k_meshlet_max_vertices);
	assert(min_triangles >= 1 && min_triangles <= max_triangles && max_triangles <= k_meshlet_max_triangles);

	assert(cone_weight >= 0 && cone_weight <= 1);
	assert(split_factor >= 0);

	if (index_count == 0)
		return 0;

	array_stack_heap_allocator<24> allocator;

	triangle_adjacency adjacency = {};
	if (vertex_count > index_count && index_count < (1u << 31))
		build_triangle_adjacency_sparse(adjacency, indices, index_count, vertex_count, allocator);
	else
		build_triangle_adjacency(adjacency, indices, index_count, vertex_count, allocator);

	unsigned int* live_triangles = adjacency.counts.data();

	size_t face_count = index_count / 3;

	unsigned char* emitted_flags = allocator.allocate<unsigned char>(face_count);
	memset(emitted_flags, 0, face_count);

	surface_cone* triangle_cones = allocator.allocate<surface_cone>(face_count);
	float mesh_area = compute_triangle_cones(triangle_cones, indices, index_count, vertex_positions, vertex_count, vertex_positions_stride);

	float triangle_area_avg = face_count == 0 ? 0.f : mesh_area / float(face_count) * 0.5f;
	float meshlet_expected_radius = sqrtf(triangle_area_avg * max_triangles) * 0.5f;

	unsigned int* kd_tree_primitive_indices = allocator.allocate<unsigned int>(face_count);
	for (size_t i = 0; i < face_count; ++i)
		kd_tree_primitive_indices[i] = unsigned(i);

	kd_tree_node* kd_tree_nodes = allocator.allocate<kd_tree_node>(face_count * 2);
	kdtree_build(0, kd_tree_nodes, face_count * 2, &triangle_cones[0].center.x, sizeof(surface_cone) / sizeof(float), kd_tree_primitive_indices, face_count, 8, 0);

	float scene_min_x = FLT_MAX, scene_min_y = FLT_MAX, scene_min_z = FLT_MAX;

	for (size_t i = 0; i < face_count; ++i) {
		const surface_cone& triangle_cone = triangle_cones[i];

		scene_min_x = scene_min_x > triangle_cone.center.x ? triangle_cone.center.x : scene_min_x;
		scene_min_y = scene_min_y > triangle_cone.center.y ? triangle_cone.center.y : scene_min_y;
		scene_min_z = scene_min_z > triangle_cone.center.z ? triangle_cone.center.z : scene_min_z;
	}

	short* used = allocator.allocate<short>(vertex_count);
	clear_used(used, vertex_count, indices, index_count);

	unsigned int initial_seed = ~0u;
	float initial_score = FLT_MAX;

	for (size_t i = 0; i < face_count; ++i) {
		const surface_cone& triangle_cone = triangle_cones[i];

		float dx = triangle_cone.center.x - scene_min_x, dy = triangle_cone.center.y - scene_min_y, dz = triangle_cone.center.z - scene_min_z;
		float score = sqrtf(dx * dx + dy * dy + dz * dz);

		if (initial_seed == ~0u || score < initial_score) {
			initial_seed = unsigned(i);
			initial_score = score;
		}
	}

	unsigned int seeds[k_meshlet_max_seeds] = {};
	size_t seed_count = 0;

	mesh_meshlet meshlet = {};
	size_t meshlet_offset = 0;

	surface_cone meshlet_cone_sum = {};

	for (;;) {
		surface_cone current_meshlet_cone = get_meshlet_cone(meshlet_cone_sum, meshlet.triangle_count);

		unsigned int best_triangle = ~0u;

		if (meshlet_offset == 0 && meshlet.triangle_count == 0)
			best_triangle = initial_seed;
		else
			best_triangle = get_neighbor_triangle(meshlet, current_meshlet_cone, meshlet_vertices, indices, adjacency, triangle_cones, live_triangles, used, meshlet_expected_radius, cone_weight);

		bool split = false;

		if (best_triangle == ~0u) {
			float query_position[3] = {current_meshlet_cone.center.x, current_meshlet_cone.center.y, current_meshlet_cone.center.z};
			unsigned int nearest_triangle = ~0u;
			float distance = FLT_MAX;

			kdtree_nearest(kd_tree_nodes, 0, &triangle_cones[0].center.x, sizeof(surface_cone) / sizeof(float), emitted_flags, query_position, nearest_triangle, distance);

			best_triangle = nearest_triangle;
			split = meshlet.triangle_count >= min_triangles && split_factor > 0 && distance > meshlet_expected_radius * split_factor;
		}

		if (best_triangle == ~0u)
			break;

		int best_extra = (used[indices[best_triangle * 3 + 0]] < 0) + (used[indices[best_triangle * 3 + 1]] < 0) + (used[indices[best_triangle * 3 + 2]] < 0);

		if (split || (meshlet.vertex_count + best_extra > max_vertices || meshlet.triangle_count >= max_triangles)) {
			seed_count = prune_seed_triangles(seeds, seed_count, emitted_flags);
			seed_count = (seed_count + k_meshlet_add_seeds <= k_meshlet_max_seeds) ? seed_count : k_meshlet_max_seeds - k_meshlet_add_seeds;
			seed_count += append_seed_triangles(seeds + seed_count, meshlet, meshlet_vertices, indices, adjacency, triangle_cones, live_triangles, scene_min_x, scene_min_y, scene_min_z);

			unsigned int best_seed = select_seed_triangle(seeds, seed_count, indices, triangle_cones, live_triangles, scene_min_x, scene_min_y, scene_min_z);

			best_triangle = best_seed != ~0u ? best_seed : best_triangle;
		}

		unsigned int a = indices[best_triangle * 3 + 0], b = indices[best_triangle * 3 + 1], c = indices[best_triangle * 3 + 2];
		assert(a < vertex_count && b < vertex_count && c < vertex_count);

		if (append_meshlet(meshlet, a, b, c, used, meshlets, meshlet_vertices, meshlet_triangles, meshlet_offset, max_vertices, max_triangles, split)) {
			meshlet_offset++;
			memset(&meshlet_cone_sum, 0, sizeof(meshlet_cone_sum));
		}

		for (size_t k = 0; k < 3; ++k) {
			unsigned int index = indices[best_triangle * 3 + k];

			unsigned int* neighbors = adjacency.triangles.data() + adjacency.offsets[index];
			size_t neighbors_size = adjacency.counts[index];

			for (size_t i = 0; i < neighbors_size; ++i) {
				unsigned int tri = neighbors[i];

				if (tri == best_triangle) {
					neighbors[i] = neighbors[neighbors_size - 1];
					adjacency.counts[index]--;
					break;
				}
			}
		}

		meshlet_cone_sum.center.x += triangle_cones[best_triangle].center.x;
		meshlet_cone_sum.center.y += triangle_cones[best_triangle].center.y;
		meshlet_cone_sum.center.z += triangle_cones[best_triangle].center.z;
		meshlet_cone_sum.axis.x += triangle_cones[best_triangle].axis.x;
		meshlet_cone_sum.axis.y += triangle_cones[best_triangle].axis.y;
		meshlet_cone_sum.axis.z += triangle_cones[best_triangle].axis.z;

		assert(!emitted_flags[best_triangle]);
		emitted_flags[best_triangle] = 1;
	}

	if (meshlet.triangle_count)
		meshlets[meshlet_offset++] = meshlet;

	assert(meshlet_offset <= mesh_build_meshlets_bound(index_count, max_vertices, min_triangles));
	assert(meshlet.triangle_offset + meshlet.triangle_count * 3 <= index_count && meshlet.vertex_offset + meshlet.vertex_count <= index_count);
	return meshlet_offset;
}

size_t mesh_build_meshlets(mesh_meshlet* meshlets, unsigned int* meshlet_vertices, unsigned char* meshlet_triangles, const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, size_t max_vertices, size_t max_triangles, float cone_weight) {
	return mesh_build_meshlets_flex(meshlets, meshlet_vertices, meshlet_triangles, indices, index_count, vertex_positions, vertex_count, vertex_positions_stride, max_vertices, max_triangles, max_triangles, cone_weight, 0.0f);
}

size_t mesh_build_meshlets_scan(mesh_meshlet* meshlets, unsigned int* meshlet_vertices, unsigned char* meshlet_triangles, const unsigned int* indices, size_t index_count, size_t vertex_count, size_t max_vertices, size_t max_triangles) {
	using namespace mars::mesh::core;

	assert(index_count % 3 == 0);

	assert(max_vertices >= 3 && max_vertices <= k_meshlet_max_vertices);
	assert(max_triangles >= 1 && max_triangles <= k_meshlet_max_triangles);

	array_stack_heap_allocator<24> allocator;

	short* used = allocator.allocate<short>(vertex_count);
	clear_used(used, vertex_count, indices, index_count);

	mesh_meshlet meshlet = {};
	size_t meshlet_offset = 0;

	for (size_t i = 0; i < index_count; i += 3) {
		unsigned int a = indices[i + 0], b = indices[i + 1], c = indices[i + 2];
		assert(a < vertex_count && b < vertex_count && c < vertex_count);

		meshlet_offset += append_meshlet(meshlet, a, b, c, used, meshlets, meshlet_vertices, meshlet_triangles, meshlet_offset, max_vertices, max_triangles);
	}

	if (meshlet.triangle_count)
		meshlets[meshlet_offset++] = meshlet;

	assert(meshlet_offset <= mesh_build_meshlets_bound(index_count, max_vertices, max_triangles));
	assert(meshlet.triangle_offset + meshlet.triangle_count * 3 <= index_count && meshlet.vertex_offset + meshlet.vertex_count <= index_count);
	return meshlet_offset;
}

size_t mesh_build_meshlets_spatial(struct mesh_meshlet* meshlets, unsigned int* meshlet_vertices, unsigned char* meshlet_triangles, const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, size_t max_vertices, size_t min_triangles, size_t max_triangles, float fill_weight) {
	using namespace mars::mesh::core;

	assert(index_count % 3 == 0);
	assert(vertex_positions_stride >= 12 && vertex_positions_stride <= 256);
	assert(vertex_positions_stride % sizeof(float) == 0);

	assert(max_vertices >= 3 && max_vertices <= k_meshlet_max_vertices);
	assert(min_triangles >= 1 && min_triangles <= max_triangles && max_triangles <= k_meshlet_max_triangles);

	if (index_count == 0)
		return 0;

	size_t face_count = index_count / 3;
	size_t vertex_stride_float = vertex_positions_stride / sizeof(float);

	array_stack_heap_allocator<24> allocator;

	float* scratch = allocator.allocate<float>(face_count * 4);

	bvh_bounds* triangle_bounds = allocator.allocate<bvh_bounds>(face_count + 1);
	bvh_prepare(triangle_bounds, scratch, indices, face_count, vertex_positions, vertex_count, vertex_stride_float);
	memset(triangle_bounds + face_count, 0, sizeof(bvh_bounds));

	unsigned int* axes = allocator.allocate<unsigned int>(face_count * 3);
	unsigned int* temp = reinterpret_cast<unsigned int*>(scratch) + face_count * 3;

	for (int k = 0; k < 3; ++k) {
		unsigned int* order = axes + k * face_count;
		const float* keys = scratch + k * face_count;

		unsigned int hist[1024][3];
		compute_histogram(hist, keys, face_count);

		for (size_t i = 0; i < face_count; ++i)
			temp[i] = unsigned(i);

		radix_pass(order, temp, keys, face_count, hist, 0);
		radix_pass(temp, order, keys, face_count, hist, 1);
		radix_pass(order, temp, keys, face_count, hist, 2);
	}

	short* used = allocator.allocate<short>(vertex_count);
	clear_used(used, vertex_count, indices, index_count);

	unsigned char* boundary = allocator.allocate<unsigned char>(face_count);

	bvh_split(triangle_bounds, &axes[0], &axes[face_count], &axes[face_count * 2], boundary, face_count, 0, scratch, used, indices, max_vertices, min_triangles, max_triangles, fill_weight);

	size_t meshlet_count = 0;
	for (size_t i = 0; i < face_count; ++i) {
		assert(boundary[i] <= 1);
		meshlet_count += boundary[i];
	}

	size_t meshlet_bound = mesh_build_meshlets_bound(index_count, max_vertices, min_triangles);

	mesh_meshlet meshlet = {};
	size_t meshlet_offset = 0;
	size_t meshlet_pending = meshlet_count;

	for (size_t i = 0; i < face_count; ++i) {
		assert(boundary[i] <= 1);
		bool split = i > 0 && boundary[i] == 1;

		if (split && meshlet_count > meshlet_bound && meshlet_offset + meshlet_pending >= meshlet_bound)
			split = false;

		unsigned int index = axes[i];
		assert(index < face_count);

		unsigned int a = indices[index * 3 + 0], b = indices[index * 3 + 1], c = indices[index * 3 + 2];

		meshlet_offset += append_meshlet(meshlet, a, b, c, used, meshlets, meshlet_vertices, meshlet_triangles, meshlet_offset, max_vertices, max_triangles, split);
		meshlet_pending -= boundary[i];
	}

	if (meshlet.triangle_count)
		meshlets[meshlet_offset++] = meshlet;

	assert(meshlet_offset <= meshlet_bound);
	assert(meshlet.triangle_offset + meshlet.triangle_count * 3 <= index_count && meshlet.vertex_offset + meshlet.vertex_count <= index_count);
	return meshlet_offset;
}

mesh_bounds mesh_compute_cluster_bounds(const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride) {
	using namespace mars::mesh::core;

	assert(index_count % 3 == 0);
	assert(index_count / 3 <= k_meshlet_max_triangles);
	assert(vertex_positions_stride >= 12 && vertex_positions_stride <= 256);
	assert(vertex_positions_stride % sizeof(float) == 0);

	(void)vertex_count;

	size_t vertex_stride_float = vertex_positions_stride / sizeof(float);

	float normals[k_meshlet_max_triangles][3];
	float corners[k_meshlet_max_triangles][3][3];
	size_t triangles = 0;

	for (size_t i = 0; i < index_count; i += 3) {
		unsigned int a = indices[i + 0], b = indices[i + 1], c = indices[i + 2];
		assert(a < vertex_count && b < vertex_count && c < vertex_count);

		const float* p0 = vertex_positions + vertex_stride_float * a;
		const float* p1 = vertex_positions + vertex_stride_float * b;
		const float* p2 = vertex_positions + vertex_stride_float * c;

		float p10[3] = {p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]};
		float p20[3] = {p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]};

		float normalx = p10[1] * p20[2] - p10[2] * p20[1];
		float normaly = p10[2] * p20[0] - p10[0] * p20[2];
		float normalz = p10[0] * p20[1] - p10[1] * p20[0];

		float area = sqrtf(normalx * normalx + normaly * normaly + normalz * normalz);

		if (area == 0.f)
			continue;

		normals[triangles][0] = normalx / area;
		normals[triangles][1] = normaly / area;
		normals[triangles][2] = normalz / area;
		memcpy(corners[triangles][0], p0, 3 * sizeof(float));
		memcpy(corners[triangles][1], p1, 3 * sizeof(float));
		memcpy(corners[triangles][2], p2, 3 * sizeof(float));
		triangles++;
	}

	mesh_bounds bounds = {};

	if (triangles == 0)
		return bounds;

	const float rzero = 0.f;

	float psphere[4] = {};
	compute_bounding_sphere(psphere, corners[0][0], triangles * 3, sizeof(float) * 3, &rzero, 0, 7);

	float center[3] = {psphere[0], psphere[1], psphere[2]};

	float nsphere[4] = {};
	compute_bounding_sphere(nsphere, normals[0], triangles, sizeof(float) * 3, &rzero, 0, 3);

	float axis[3] = {nsphere[0], nsphere[1], nsphere[2]};
	float axislength = sqrtf(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
	float invaxislength = axislength == 0.f ? 0.f : 1.f / axislength;

	axis[0] *= invaxislength;
	axis[1] *= invaxislength;
	axis[2] *= invaxislength;

	float mindp = 1.f;

	for (size_t i = 0; i < triangles; ++i) {
		float dp = normals[i][0] * axis[0] + normals[i][1] * axis[1] + normals[i][2] * axis[2];

		mindp = (dp < mindp) ? dp : mindp;
	}

	bounds.center[0] = center[0];
	bounds.center[1] = center[1];
	bounds.center[2] = center[2];
	bounds.radius = psphere[3];

	if (mindp <= 0.1f) {
		bounds.cone_cutoff = 1;
		bounds.cone_cutoff_s8 = 127;
		return bounds;
	}

	float maxt = 0;

	for (size_t i = 0; i < triangles; ++i) {
		float cx = center[0] - corners[i][0][0];
		float cy = center[1] - corners[i][0][1];
		float cz = center[2] - corners[i][0][2];

		float dc = cx * normals[i][0] + cy * normals[i][1] + cz * normals[i][2];
		float dn = axis[0] * normals[i][0] + axis[1] * normals[i][1] + axis[2] * normals[i][2];

		assert(dn > 0.f);
		float t = dc / dn;

		maxt = (t > maxt) ? t : maxt;
	}

	bounds.cone_apex[0] = center[0] - axis[0] * maxt;
	bounds.cone_apex[1] = center[1] - axis[1] * maxt;
	bounds.cone_apex[2] = center[2] - axis[2] * maxt;

	bounds.cone_axis[0] = axis[0];
	bounds.cone_axis[1] = axis[1];
	bounds.cone_axis[2] = axis[2];

	bounds.cone_cutoff = sqrtf(1 - mindp * mindp);

	bounds.cone_axis_s8[0] = (signed char)(mesh_quantize_snorm(bounds.cone_axis[0], 8));
	bounds.cone_axis_s8[1] = (signed char)(mesh_quantize_snorm(bounds.cone_axis[1], 8));
	bounds.cone_axis_s8[2] = (signed char)(mesh_quantize_snorm(bounds.cone_axis[2], 8));

	float cone_axis_s8_e0 = fabsf(bounds.cone_axis_s8[0] / 127.f - bounds.cone_axis[0]);
	float cone_axis_s8_e1 = fabsf(bounds.cone_axis_s8[1] / 127.f - bounds.cone_axis[1]);
	float cone_axis_s8_e2 = fabsf(bounds.cone_axis_s8[2] / 127.f - bounds.cone_axis[2]);

	int cone_cutoff_s8 = int(127 * (bounds.cone_cutoff + cone_axis_s8_e0 + cone_axis_s8_e1 + cone_axis_s8_e2) + 1);

	bounds.cone_cutoff_s8 = (cone_cutoff_s8 > 127) ? 127 : (signed char)(cone_cutoff_s8);

	return bounds;
}

mesh_bounds mesh_compute_meshlet_bounds(const unsigned int* meshlet_vertices, const unsigned char* meshlet_triangles, size_t triangle_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride) {
	using namespace mars::mesh::core;

	assert(triangle_count <= k_meshlet_max_triangles);
	assert(vertex_positions_stride >= 12 && vertex_positions_stride <= 256);
	assert(vertex_positions_stride % sizeof(float) == 0);

	unsigned int indices[k_meshlet_max_triangles * 3];

	for (size_t i = 0; i < triangle_count * 3; ++i) {
		unsigned int index = meshlet_vertices[meshlet_triangles[i]];
		assert(index < vertex_count);

		indices[i] = index;
	}

	return mesh_compute_cluster_bounds(indices, triangle_count * 3, vertex_positions, vertex_count, vertex_positions_stride);
}

mesh_bounds mesh_compute_sphere_bounds(const float* positions, size_t count, size_t positions_stride, const float* radii, size_t radii_stride) {
	using namespace mars::mesh::core;

	assert(positions_stride >= 12 && positions_stride <= 256);
	assert(positions_stride % sizeof(float) == 0);
	assert((radii_stride >= 4 && radii_stride <= 256) || radii == NULL);
	assert(radii_stride % sizeof(float) == 0);

	mesh_bounds bounds = {};

	if (count == 0)
		return bounds;

	const float rzero = 0.f;

	float psphere[4] = {};
	compute_bounding_sphere(psphere, positions, count, positions_stride, radii ? radii : &rzero, radii ? radii_stride : 0, 7);

	bounds.center[0] = psphere[0];
	bounds.center[1] = psphere[1];
	bounds.center[2] = psphere[2];
	bounds.radius = psphere[3];

	return bounds;
}

void mesh_optimize_meshlet(unsigned int* meshlet_vertices, unsigned char* meshlet_triangles, size_t triangle_count, size_t vertex_count) {
	using namespace mars::mesh::core;

	assert(triangle_count <= k_meshlet_max_triangles);
	assert(vertex_count <= k_meshlet_max_vertices);

	unsigned char* indices = meshlet_triangles;
	unsigned int* vertices = meshlet_vertices;

	unsigned char cache[k_meshlet_max_vertices];
	memset(cache, 0, vertex_count);

	unsigned char cache_last = 128;
	const unsigned char cache_cutoff = 3;

	for (size_t i = 0; i < triangle_count; ++i) {
		int next = -1;
		int next_match = -1;

		for (size_t j = i; j < triangle_count; ++j) {
			unsigned char a = indices[j * 3 + 0], b = indices[j * 3 + 1], c = indices[j * 3 + 2];
			assert(a < vertex_count && b < vertex_count && c < vertex_count);

			int aok = (unsigned char)(cache_last - cache[a]) < cache_cutoff;
			int bok = (unsigned char)(cache_last - cache[b]) < cache_cutoff;
			int cok = (unsigned char)(cache_last - cache[c]) < cache_cutoff;

			if (aok + bok + cok > next_match) {
				next = (int)j;
				next_match = aok + bok + cok;

				if (next_match >= 2)
					break;
			}
		}

		assert(next >= 0);

		unsigned char a = indices[next * 3 + 0], b = indices[next * 3 + 1], c = indices[next * 3 + 2];

		memmove(indices + (i + 1) * 3, indices + i * 3, (next - i) * 3 * sizeof(unsigned char));

		indices[i * 3 + 0] = a;
		indices[i * 3 + 1] = b;
		indices[i * 3 + 2] = c;

		cache_last++;
		cache[a] = cache_last;
		cache[b] = cache_last;
		cache[c] = cache_last;
	}

	unsigned int order[k_meshlet_max_vertices];

	short remap[k_meshlet_max_vertices];
	memset(remap, -1, vertex_count * sizeof(short));

	size_t vertex_offset = 0;

	for (size_t i = 0; i < triangle_count * 3; ++i) {
		short& r = remap[indices[i]];

		if (r < 0) {
			r = short(vertex_offset);
			order[vertex_offset] = vertices[indices[i]];
			vertex_offset++;
		}

		indices[i] = (unsigned char)r;
	}

	assert(vertex_offset <= vertex_count);
	memcpy(vertices, order, vertex_offset * sizeof(unsigned int));
}

#undef SIMD_SSE
#undef SIMD_NEON

} // namespace mars::mesh
