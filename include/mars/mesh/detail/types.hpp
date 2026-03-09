#pragma once

#include <mars/math/vector3.hpp>
#include <mars/math/vector4.hpp>

#include <cstddef>

namespace mars::mesh::core {

struct vertex_edge_adjacency {
	struct adjacency_edge {
		unsigned int next;
		unsigned int prev;
	};

	unsigned int* offsets;
	adjacency_edge* data;
};

struct position_key_hasher {
	const float* vertex_positions;
	size_t vertex_stride_float;
	const unsigned int* sparse_remap;

	size_t hash(unsigned int index) const;
	bool equal(unsigned int lhs, unsigned int rhs) const;
};

struct remap_value_hasher {
	unsigned int* remap;

	size_t hash(unsigned int id) const;
	bool equal(unsigned int lhs, unsigned int rhs) const;
};

struct error_quadric {
	mars::vector3<float> diagonal = {};
	mars::vector3<float> off_diagonal = {};
	mars::vector3<float> linear = {};
	float constant_term = 0.0f;
	float weight = 0.0f;
};

struct quadric_gradient {
	mars::vector4<float> coefficients = {};
};

struct point_reservoir {
	mars::vector3<float> position = {};
	mars::vector3<float> color = {};
	float weight = 0.0f;
};

struct edge_collapse {
	unsigned int v0;
	unsigned int v1;
	bool bidirectional = false;
	float error = 0.0f;
};

struct vertex_cell_hasher {
	const unsigned int* vertex_ids;

	size_t hash(unsigned int i) const;
	bool equal(unsigned int lhs, unsigned int rhs) const;
};

struct integer_id_hasher {
	size_t hash(unsigned int id) const;
	bool equal(unsigned int lhs, unsigned int rhs) const;
};

struct triangle_index_triplet_hasher {
	const unsigned int* indices;

	size_t hash(unsigned int i) const;
	bool equal(unsigned int lhs, unsigned int rhs) const;
};

} // namespace mars::mesh::core
