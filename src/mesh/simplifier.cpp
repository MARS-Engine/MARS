#include <mars/mesh/mesh.hpp>
#include <mars/mesh/detail/types.hpp>

#include <array>
#include <assert.h>
#include <bit>
#include <float.h>
#include <math.h>
#include <string.h>

#ifndef TRACE
#define TRACE 0
#endif

#if TRACE
#include <stdio.h>
#endif

#if TRACE
#define TRACESTATS(i) stats[i]++;
#else
#define TRACESTATS(i) (void)0
#endif

namespace mars::mesh {

namespace core {

static void prepare_edge_adjacency(vertex_edge_adjacency& adjacency, size_t index_count, size_t vertex_count, array_stack_heap_allocator<24>& allocator) {
	adjacency.offsets = allocator.allocate<unsigned int>(vertex_count + 1);
	adjacency.data = allocator.allocate<vertex_edge_adjacency::adjacency_edge>(index_count);
}

static void update_edge_adjacency(vertex_edge_adjacency& adjacency, const unsigned int* indices, size_t index_count, size_t vertex_count, const unsigned int* remap) {
	size_t face_count = index_count / 3;
	unsigned int* offsets = adjacency.offsets + 1;
	vertex_edge_adjacency::adjacency_edge* data = adjacency.data;

	memset(offsets, 0, vertex_count * sizeof(unsigned int));

	for (size_t i = 0; i < index_count; ++i) {
		unsigned int v = remap ? remap[indices[i]] : indices[i];
		assert(v < vertex_count);

		offsets[v]++;
	}

	unsigned int offset = 0;

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int count = offsets[i];
		offsets[i] = offset;
		offset += count;
	}

	assert(offset == index_count);

	for (size_t i = 0; i < face_count; ++i) {
		unsigned int a = indices[i * 3 + 0], b = indices[i * 3 + 1], c = indices[i * 3 + 2];

		if (remap) {
			a = remap[a];
			b = remap[b];
			c = remap[c];
		}

		data[offsets[a]].next = b;
		data[offsets[a]].prev = c;
		offsets[a]++;

		data[offsets[b]].next = c;
		data[offsets[b]].prev = a;
		offsets[b]++;

		data[offsets[c]].next = a;
		data[offsets[c]].prev = b;
		offsets[c]++;
	}

	adjacency.offsets[0] = 0;
	assert(adjacency.offsets[vertex_count] == index_count);
}

size_t position_key_hasher::hash(unsigned int index) const {
	unsigned int ri = sparse_remap ? sparse_remap[index] : index;
	const unsigned int* key = reinterpret_cast<const unsigned int*>(vertex_positions + ri * vertex_stride_float);

	unsigned int x = key[0], y = key[1], z = key[2];

	x = (x == 0x80000000) ? 0 : x;
	y = (y == 0x80000000) ? 0 : y;
	z = (z == 0x80000000) ? 0 : z;

	x ^= x >> 17;
	y ^= y >> 17;
	z ^= z >> 17;

	return (x * 73856093) ^ (y * 19349663) ^ (z * 83492791);
}

bool position_key_hasher::equal(unsigned int lhs, unsigned int rhs) const {
	unsigned int li = sparse_remap ? sparse_remap[lhs] : lhs;
	unsigned int ri = sparse_remap ? sparse_remap[rhs] : rhs;

	const float* lv = vertex_positions + li * vertex_stride_float;
	const float* rv = vertex_positions + ri * vertex_stride_float;

	return lv[0] == rv[0] && lv[1] == rv[1] && lv[2] == rv[2];
}

size_t remap_value_hasher::hash(unsigned int id) const {
	return id * 0x5bd1e995;
}

bool remap_value_hasher::equal(unsigned int lhs, unsigned int rhs) const {
	return remap[lhs] == rhs;
}

static size_t hash_buckets2(size_t count) {
	size_t buckets = 1;
	while (buckets < count + count / 4)
		buckets *= 2;

	return buckets;
}

template <typename T, typename hash_type>
static T* hash_lookup2(T* table, size_t buckets, const hash_type& hasher, const T& key, const T& empty) {
	assert(buckets > 0);
	assert((buckets & (buckets - 1)) == 0);

	size_t hashmod = buckets - 1;
	size_t bucket = hasher.hash(key) & hashmod;

	for (size_t probe = 0; probe <= hashmod; ++probe) {
		T& item = table[bucket];

		if (item == empty)
			return &item;

		if (hasher.equal(item, key))
			return &item;

		bucket = (bucket + probe + 1) & hashmod;
	}

	assert(false && "hash table is full");
	return NULL;
}

static void build_position_remap(unsigned int* remap, unsigned int* wedge, const float* vertex_positions_data, size_t vertex_count, size_t vertex_positions_stride, const unsigned int* sparse_remap, array_stack_heap_allocator<24>& allocator) {
	position_key_hasher hasher = {vertex_positions_data, vertex_positions_stride / sizeof(float), sparse_remap};

	size_t table_size = hash_buckets2(vertex_count);
	unsigned int* table = allocator.allocate<unsigned int>(table_size);
	memset(table, -1, table_size * sizeof(unsigned int));

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int index = unsigned(i);
		unsigned int* entry = hash_lookup2(table, table_size, hasher, index, ~0u);

		if (*entry == ~0u)
			*entry = index;

		remap[index] = *entry;
	}

	allocator.deallocate(table);

	if (!wedge)
		return;

	for (size_t i = 0; i < vertex_count; ++i)
		wedge[i] = unsigned(i);

	for (size_t i = 0; i < vertex_count; ++i)
		if (remap[i] != i) {
			unsigned int r = remap[i];

			wedge[i] = wedge[r];
			wedge[r] = unsigned(i);
		}
}

static unsigned int* build_sparse_remap(unsigned int* indices, size_t index_count, size_t vertex_count, size_t* out_vertex_count, array_stack_heap_allocator<24>& allocator) {

	unsigned char* filter = allocator.allocate<unsigned char>((vertex_count + 7) / 8);

	for (size_t i = 0; i < index_count; ++i) {
		unsigned int index = indices[i];
		assert(index < vertex_count);
		filter[index / 8] = 0;
	}

	size_t unique = 0;
	for (size_t i = 0; i < index_count; ++i) {
		unsigned int index = indices[i];
		unique += (filter[index / 8] & (1 << (index % 8))) == 0;
		filter[index / 8] |= 1 << (index % 8);
	}

	unsigned int* remap = allocator.allocate<unsigned int>(unique);
	size_t offset = 0;

	size_t revremap_size = hash_buckets2(unique);
	unsigned int* revremap = allocator.allocate<unsigned int>(revremap_size);
	memset(revremap, -1, revremap_size * sizeof(unsigned int));

	remap_value_hasher hasher = {remap};

	for (size_t i = 0; i < index_count; ++i) {
		unsigned int index = indices[i];
		unsigned int* entry = hash_lookup2(revremap, revremap_size, hasher, index, ~0u);

		if (*entry == ~0u) {
			remap[offset] = index;
			*entry = unsigned(offset);
			offset++;
		}

		indices[i] = *entry;
	}

	allocator.deallocate(revremap);

	assert(offset == unique);
	*out_vertex_count = unique;

	return remap;
}

enum mars_vertex_type {
	MARS_VERTEX_TYPE_MANIFOLD,
	MARS_VERTEX_TYPE_BORDER,
	MARS_VERTEX_TYPE_SEAM,
	MARS_VERTEX_TYPE_COMPLEX,
	MARS_VERTEX_TYPE_LOCKED,

	MARS_VERTEX_TYPE_COUNT
};

constexpr std::array<std::array<unsigned char, MARS_VERTEX_TYPE_COUNT>, MARS_VERTEX_TYPE_COUNT> k_can_collapse = {{
    {{1, 1, 1, 1, 1}},
    {{0, 1, 0, 0, 1}},
    {{0, 0, 1, 0, 1}},
    {{0, 0, 0, 1, 1}},
    {{0, 0, 0, 0, 0}},
}};

constexpr std::array<std::array<unsigned char, MARS_VERTEX_TYPE_COUNT>, MARS_VERTEX_TYPE_COUNT> k_has_opposite = {{
    {{1, 1, 1, 1, 1}},
    {{1, 0, 1, 0, 0}},
    {{1, 1, 1, 0, 1}},
    {{1, 0, 0, 0, 0}},
    {{1, 0, 1, 0, 0}},
}};

static bool has_edge(const vertex_edge_adjacency& adjacency, unsigned int a, unsigned int b) {
	unsigned int count = adjacency.offsets[a + 1] - adjacency.offsets[a];
	const vertex_edge_adjacency::adjacency_edge* edges = adjacency.data + adjacency.offsets[a];

	for (size_t i = 0; i < count; ++i)
		if (edges[i].next == b)
			return true;

	return false;
}

static bool has_edge(const vertex_edge_adjacency& adjacency, unsigned int a, unsigned int b, const unsigned int* remap, const unsigned int* wedge) {
	unsigned int v = a;

	do {
		unsigned int count = adjacency.offsets[v + 1] - adjacency.offsets[v];
		const vertex_edge_adjacency::adjacency_edge* edges = adjacency.data + adjacency.offsets[v];

		for (size_t i = 0; i < count; ++i)
			if (remap[edges[i].next] == remap[b])
				return true;

		v = wedge[v];
	} while (v != a);

	return false;
}

static void classify_vertices(unsigned char* result, unsigned int* loop, unsigned int* loopback, size_t vertex_count, const vertex_edge_adjacency& adjacency, const unsigned int* remap, const unsigned int* wedge, const unsigned char* vertex_lock, const unsigned int* sparse_remap, unsigned int options) {
	memset(loop, -1, vertex_count * sizeof(unsigned int));
	memset(loopback, -1, vertex_count * sizeof(unsigned int));

	unsigned int* openinc = loopback;
	unsigned int* openout = loop;

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int vertex = unsigned(i);

		unsigned int count = adjacency.offsets[vertex + 1] - adjacency.offsets[vertex];
		const vertex_edge_adjacency::adjacency_edge* edges = adjacency.data + adjacency.offsets[vertex];

		for (size_t j = 0; j < count; ++j) {
			unsigned int target = edges[j].next;

			if (target == vertex) {

				openinc[vertex] = openout[vertex] = vertex;
			} else if (!has_edge(adjacency, target, vertex)) {
				openinc[target] = (openinc[target] == ~0u) ? vertex : target;
				openout[vertex] = (openout[vertex] == ~0u) ? target : vertex;
			}
		}
	}

#if TRACE
	size_t stats[4] = {};
#endif

	for (size_t i = 0; i < vertex_count; ++i) {
		if (remap[i] == i) {
			if (wedge[i] == i) {

				unsigned int openi = openinc[i], openo = openout[i];

				if (openi == ~0u && openo == ~0u) {
					result[i] = MARS_VERTEX_TYPE_MANIFOLD;
				} else if (openi != ~0u && openo != ~0u && remap[openi] == remap[openo] && openi != i) {

					result[i] = MARS_VERTEX_TYPE_SEAM;
				} else if (openi != i && openo != i) {
					result[i] = MARS_VERTEX_TYPE_BORDER;
				} else {
					result[i] = MARS_VERTEX_TYPE_LOCKED;
					TRACESTATS(0);
				}
			} else if (wedge[wedge[i]] == i) {

				unsigned int w = wedge[i];
				unsigned int openiv = openinc[i], openov = openout[i];
				unsigned int openiw = openinc[w], openow = openout[w];

				if (openiv != ~0u && openiv != i && openov != ~0u && openov != i &&
				    openiw != ~0u && openiw != w && openow != ~0u && openow != w) {
					if (remap[openiv] == remap[openow] && remap[openov] == remap[openiw] && remap[openiv] != remap[openov]) {
						result[i] = MARS_VERTEX_TYPE_SEAM;
					} else {
						result[i] = MARS_VERTEX_TYPE_LOCKED;
						TRACESTATS(1);
					}
				} else {
					result[i] = MARS_VERTEX_TYPE_LOCKED;
					TRACESTATS(2);
				}
			} else {

				result[i] = MARS_VERTEX_TYPE_LOCKED;
				TRACESTATS(3);
			}
		} else {
			assert(remap[i] < i);

			result[i] = result[remap[i]];
		}
	}

	if (options & mesh_simplify_permissive_flag)
		for (size_t i = 0; i < vertex_count; ++i)
			if (result[i] == MARS_VERTEX_TYPE_SEAM || result[i] == MARS_VERTEX_TYPE_LOCKED) {
				if (remap[i] != i) {

					result[i] = result[remap[i]];
					continue;
				}

				bool protect = false;

				unsigned int v = unsigned(i);
				do {
					unsigned int rv = sparse_remap ? sparse_remap[v] : v;
					protect |= vertex_lock && (vertex_lock[rv] & mesh_simplify_vertex_protect) != 0;
					v = wedge[v];
				} while (v != i);

				do {
					const vertex_edge_adjacency::adjacency_edge* edges = &adjacency.data[adjacency.offsets[v]];
					size_t count = adjacency.offsets[v + 1] - adjacency.offsets[v];

					for (size_t j = 0; j < count; ++j)
						protect |= !has_edge(adjacency, edges[j].next, v, remap, wedge);
					v = wedge[v];
				} while (v != i);

				result[i] = protect ? result[i] : int(MARS_VERTEX_TYPE_COMPLEX);
			}

	if (vertex_lock) {

		for (size_t i = 0; i < vertex_count; ++i) {
			unsigned int ri = sparse_remap ? sparse_remap[i] : unsigned(i);

			if (vertex_lock[ri] & mesh_simplify_vertex_lock)
				result[remap[i]] = MARS_VERTEX_TYPE_LOCKED;
		}

		for (size_t i = 0; i < vertex_count; ++i)
			if (result[remap[i]] == MARS_VERTEX_TYPE_LOCKED)
				result[i] = MARS_VERTEX_TYPE_LOCKED;
	}

	if (options & mesh_simplify_lock_border_flag)
		for (size_t i = 0; i < vertex_count; ++i)
			if (result[i] == MARS_VERTEX_TYPE_BORDER)
				result[i] = MARS_VERTEX_TYPE_LOCKED;

#if TRACE
	printf("locked: many open edges %d, disconnected seam %d, many seam edges %d, many wedges %d\n",
	       int(stats[0]), int(stats[1]), int(stats[2]), int(stats[3]));
#endif
}

static float rescale_positions(mars::vector3<float>* result, const float* vertex_positions_data, size_t vertex_count, size_t vertex_positions_stride, const unsigned int* sparse_remap = NULL, float* out_offset = NULL) {
	size_t vertex_stride_float = vertex_positions_stride / sizeof(float);

	float minv[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
	float maxv[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int ri = sparse_remap ? sparse_remap[i] : unsigned(i);
		const float* v = vertex_positions_data + ri * vertex_stride_float;

		if (result) {
			result[i].x = v[0];
			result[i].y = v[1];
			result[i].z = v[2];
		}

		for (int j = 0; j < 3; ++j) {
			float vj = v[j];

			minv[j] = minv[j] > vj ? vj : minv[j];
			maxv[j] = maxv[j] < vj ? vj : maxv[j];
		}
	}

	float extent = 0.f;

	extent = (maxv[0] - minv[0]) < extent ? extent : (maxv[0] - minv[0]);
	extent = (maxv[1] - minv[1]) < extent ? extent : (maxv[1] - minv[1]);
	extent = (maxv[2] - minv[2]) < extent ? extent : (maxv[2] - minv[2]);

	if (result) {
		float scale = extent == 0 ? 0.f : 1.f / extent;

		for (size_t i = 0; i < vertex_count; ++i) {
			result[i].x = (result[i].x - minv[0]) * scale;
			result[i].y = (result[i].y - minv[1]) * scale;
			result[i].z = (result[i].z - minv[2]) * scale;
		}
	}

	if (out_offset) {
		out_offset[0] = minv[0];
		out_offset[1] = minv[1];
		out_offset[2] = minv[2];
	}

	return extent;
}

static void rescale_attributes(float* result, const float* vertex_attributes_data, size_t vertex_count, size_t vertex_attributes_stride, const float* attribute_weights, size_t attribute_count, const unsigned int* attribute_remap, const unsigned int* sparse_remap) {
	size_t vertex_attributes_stride_float = vertex_attributes_stride / sizeof(float);

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int ri = sparse_remap ? sparse_remap[i] : unsigned(i);

		for (size_t k = 0; k < attribute_count; ++k) {
			unsigned int rk = attribute_remap[k];
			float a = vertex_attributes_data[ri * vertex_attributes_stride_float + rk];

			result[i * attribute_count + k] = a * attribute_weights[rk];
		}
	}
}

static void finalize_vertices(float* vertex_positions_data, size_t vertex_positions_stride, float* vertex_attributes_data, size_t vertex_attributes_stride, const float* attribute_weights, size_t attribute_count, size_t vertex_count, const mars::vector3<float>* vertex_positions, const float* vertex_attributes, const unsigned int* sparse_remap, const unsigned int* attribute_remap, float vertex_scale, const float* vertex_offset, const unsigned char* vertex_kind, const unsigned char* vertex_update, const unsigned char* vertex_lock) {
	size_t vertex_positions_stride_float = vertex_positions_stride / sizeof(float);
	size_t vertex_attributes_stride_float = vertex_attributes_stride / sizeof(float);

	for (size_t i = 0; i < vertex_count; ++i) {
		if (!vertex_update[i])
			continue;

		unsigned int ri = sparse_remap ? sparse_remap[i] : unsigned(i);

		if (vertex_lock && (vertex_lock[ri] & mesh_simplify_vertex_lock) != 0)
			continue;

		if (vertex_kind[i] != MARS_VERTEX_TYPE_LOCKED) {
			const mars::vector3<float>& p = vertex_positions[i];
			float* v = vertex_positions_data + ri * vertex_positions_stride_float;

			v[0] = p.x * vertex_scale + vertex_offset[0];
			v[1] = p.y * vertex_scale + vertex_offset[1];
			v[2] = p.z * vertex_scale + vertex_offset[2];
		}

		if (attribute_count) {
			const float* sa = vertex_attributes + i * attribute_count;
			float* va = vertex_attributes_data + ri * vertex_attributes_stride_float;

			for (size_t k = 0; k < attribute_count; ++k) {
				unsigned int rk = attribute_remap[k];

				va[rk] = sa[k] / attribute_weights[rk];
			}
		}
	}
}

static const size_t k_max_attributes = 32;

static float normalize(mars::vector3<float>& v) {
	float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

	if (length > 0) {
		v.x /= length;
		v.y /= length;
		v.z /= length;
	}

	return length;
}

static unsigned int sortable_error_bits(float error) {
	return std::bit_cast<unsigned int>(error);
}

static void quadric_add(error_quadric& Q, const error_quadric& R) {
	Q.diagonal.x += R.diagonal.x;
	Q.diagonal.y += R.diagonal.y;
	Q.diagonal.z += R.diagonal.z;
	Q.off_diagonal.x += R.off_diagonal.x;
	Q.off_diagonal.y += R.off_diagonal.y;
	Q.off_diagonal.z += R.off_diagonal.z;
	Q.linear.x += R.linear.x;
	Q.linear.y += R.linear.y;
	Q.linear.z += R.linear.z;
	Q.constant_term += R.constant_term;
	Q.weight += R.weight;
}

static void quadric_add(quadric_gradient& G, const quadric_gradient& R) {
	G.coefficients.x += R.coefficients.x;
	G.coefficients.y += R.coefficients.y;
	G.coefficients.z += R.coefficients.z;
	G.coefficients.w += R.coefficients.w;
}

static void quadric_add(quadric_gradient* G, const quadric_gradient* R, size_t attribute_count) {
	for (size_t k = 0; k < attribute_count; ++k) {
		G[k].coefficients.x += R[k].coefficients.x;
		G[k].coefficients.y += R[k].coefficients.y;
		G[k].coefficients.z += R[k].coefficients.z;
		G[k].coefficients.w += R[k].coefficients.w;
	}
}

static float quadric_eval(const error_quadric& Q, const mars::vector3<float>& v) {
	float rx = Q.linear.x;
	float ry = Q.linear.y;
	float rz = Q.linear.z;

	rx += Q.off_diagonal.x * v.y;
	ry += Q.off_diagonal.z * v.z;
	rz += Q.off_diagonal.y * v.x;

	rx *= 2;
	ry *= 2;
	rz *= 2;

	rx += Q.diagonal.x * v.x;
	ry += Q.diagonal.y * v.y;
	rz += Q.diagonal.z * v.z;

	float r = Q.constant_term;
	r += rx * v.x;
	r += ry * v.y;
	r += rz * v.z;

	return r;
}

static float quadric_error(const error_quadric& Q, const mars::vector3<float>& v) {
	float r = quadric_eval(Q, v);
	float s = Q.weight == 0.f ? 0.f : 1.f / Q.weight;

	return fabsf(r) * s;
}

static float quadric_error(const error_quadric& Q, const quadric_gradient* G, size_t attribute_count, const mars::vector3<float>& v, const float* va) {
	float r = quadric_eval(Q, v);

	for (size_t k = 0; k < attribute_count; ++k) {
		float a = va[k];
		float g = v.x * G[k].coefficients.x + v.y * G[k].coefficients.y + v.z * G[k].coefficients.z + G[k].coefficients.w;

		r += a * (a * Q.weight - 2 * g);
	}

	return fabsf(r);
}

static void quadric_from_plane(error_quadric& Q, float a, float b, float c, float d, float w) {
	float aw = a * w;
	float bw = b * w;
	float cw = c * w;
	float dw = d * w;

	Q.diagonal = {a * aw, b * bw, c * cw};
	Q.off_diagonal = {a * bw, a * cw, b * cw};
	Q.linear = {a * dw, b * dw, c * dw};
	Q.constant_term = d * dw;
	Q.weight = w;
}

static void quadric_from_point(error_quadric& Q, float x, float y, float z, float w) {
	Q.diagonal = {w, w, w};
	Q.off_diagonal = {};
	Q.linear = {-x * w, -y * w, -z * w};
	Q.constant_term = (x * x + y * y + z * z) * w;
	Q.weight = w;
}

static void quadric_from_triangle(error_quadric& Q, const mars::vector3<float>& p0, const mars::vector3<float>& p1, const mars::vector3<float>& p2, float weight) {
	mars::vector3<float> p10 = {p1.x - p0.x, p1.y - p0.y, p1.z - p0.z};
	mars::vector3<float> p20 = {p2.x - p0.x, p2.y - p0.y, p2.z - p0.z};

	mars::vector3<float> normal = {p10.y * p20.z - p10.z * p20.y, p10.z * p20.x - p10.x * p20.z, p10.x * p20.y - p10.y * p20.x};
	float area = normalize(normal);

	float distance = normal.x * p0.x + normal.y * p0.y + normal.z * p0.z;

	quadric_from_plane(Q, normal.x, normal.y, normal.z, -distance, sqrtf(area) * weight);
}

static void quadric_from_triangle_edge(error_quadric& Q, const mars::vector3<float>& p0, const mars::vector3<float>& p1, const mars::vector3<float>& p2, float weight) {
	mars::vector3<float> p10 = {p1.x - p0.x, p1.y - p0.y, p1.z - p0.z};

	float lengthsq = p10.x * p10.x + p10.y * p10.y + p10.z * p10.z;
	float length = sqrtf(lengthsq);

	mars::vector3<float> p20 = {p2.x - p0.x, p2.y - p0.y, p2.z - p0.z};
	float p20p = p20.x * p10.x + p20.y * p10.y + p20.z * p10.z;

	mars::vector3<float> perp = {p20.x * lengthsq - p10.x * p20p, p20.y * lengthsq - p10.y * p20p, p20.z * lengthsq - p10.z * p20p};
	normalize(perp);

	float distance = perp.x * p0.x + perp.y * p0.y + perp.z * p0.z;

	quadric_from_plane(Q, perp.x, perp.y, perp.z, -distance, length * weight);
}

static void quadric_from_attributes(error_quadric& Q, quadric_gradient* G, const mars::vector3<float>& p0, const mars::vector3<float>& p1, const mars::vector3<float>& p2, const float* va0, const float* va1, const float* va2, size_t attribute_count) {

	mars::vector3<float> p10 = {p1.x - p0.x, p1.y - p0.y, p1.z - p0.z};
	mars::vector3<float> p20 = {p2.x - p0.x, p2.y - p0.y, p2.z - p0.z};

	mars::vector3<float> normal = {p10.y * p20.z - p10.z * p20.y, p10.z * p20.x - p10.x * p20.z, p10.x * p20.y - p10.y * p20.x};
	float area = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z) * 0.5f;

	float w = area;

	const mars::vector3<float>& v0 = p10;
	const mars::vector3<float>& v1 = p20;
	float d00 = v0.x * v0.x + v0.y * v0.y + v0.z * v0.z;
	float d01 = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
	float d11 = v1.x * v1.x + v1.y * v1.y + v1.z * v1.z;
	float denom = d00 * d11 - d01 * d01;
	float denomr = denom == 0 ? 0.f : 1.f / denom;

	float gx1 = (d11 * v0.x - d01 * v1.x) * denomr;
	float gx2 = (d00 * v1.x - d01 * v0.x) * denomr;
	float gy1 = (d11 * v0.y - d01 * v1.y) * denomr;
	float gy2 = (d00 * v1.y - d01 * v0.y) * denomr;
	float gz1 = (d11 * v0.z - d01 * v1.z) * denomr;
	float gz2 = (d00 * v1.z - d01 * v0.z) * denomr;

	memset(&Q, 0, sizeof(error_quadric));

	Q.weight = w;

	for (size_t k = 0; k < attribute_count; ++k) {
		float a0 = va0[k], a1 = va1[k], a2 = va2[k];

		float gx = gx1 * (a1 - a0) + gx2 * (a2 - a0);
		float gy = gy1 * (a1 - a0) + gy2 * (a2 - a0);
		float gz = gz1 * (a1 - a0) + gz2 * (a2 - a0);
		float gw = a0 - p0.x * gx - p0.y * gy - p0.z * gz;

		Q.diagonal.x += w * (gx * gx);
		Q.diagonal.y += w * (gy * gy);
		Q.diagonal.z += w * (gz * gz);

		Q.off_diagonal.x += w * (gy * gx);
		Q.off_diagonal.y += w * (gz * gx);
		Q.off_diagonal.z += w * (gz * gy);

		Q.linear.x += w * (gx * gw);
		Q.linear.y += w * (gy * gw);
		Q.linear.z += w * (gz * gw);

		Q.constant_term += w * (gw * gw);

		G[k].coefficients = {w * gx, w * gy, w * gz, w * gw};
	}
}

static void quadric_volume_gradient(quadric_gradient& G, const mars::vector3<float>& p0, const mars::vector3<float>& p1, const mars::vector3<float>& p2) {
	mars::vector3<float> p10 = {p1.x - p0.x, p1.y - p0.y, p1.z - p0.z};
	mars::vector3<float> p20 = {p2.x - p0.x, p2.y - p0.y, p2.z - p0.z};

	mars::vector3<float> normal = {p10.y * p20.z - p10.z * p20.y, p10.z * p20.x - p10.x * p20.z, p10.x * p20.y - p10.y * p20.x};
	float area = normalize(normal) * 0.5f;

	G.coefficients = {
	    normal.x * area,
	    normal.y * area,
	    normal.z * area,
	    (-p0.x * normal.x - p0.y * normal.y - p0.z * normal.z) * area,
	};
}

static bool quadric_solve(mars::vector3<float>& p, const error_quadric& Q, const quadric_gradient& GV) {

	float a00 = Q.diagonal.x, a11 = Q.diagonal.y, a22 = Q.diagonal.z;
	float a10 = Q.off_diagonal.x, a20 = Q.off_diagonal.y, a21 = Q.off_diagonal.z;
	float x0 = -Q.linear.x, x1 = -Q.linear.y, x2 = -Q.linear.z;

	float eps = 1e-6f * Q.weight;

	float d0 = a00;
	float l10 = a10 / d0;
	float l20 = a20 / d0;

	float d1 = a11 - a10 * l10;
	float dl21 = a21 - a20 * l10;
	float l21 = dl21 / d1;

	float d2 = a22 - a20 * l20 - dl21 * l21;

	float y0 = x0;
	float y1 = x1 - l10 * y0;
	float y2 = x2 - l20 * y0 - l21 * y1;

	float z0 = y0 / d0;
	float z1 = y1 / d1;
	float z2 = y2 / d2;

	float a30 = GV.coefficients.x, a31 = GV.coefficients.y, a32 = GV.coefficients.z;
	float x3 = -GV.coefficients.w;

	float l30 = a30 / d0;
	float dl31 = a31 - a30 * l10;
	float l31 = dl31 / d1;
	float dl32 = a32 - a30 * l20 - dl31 * l21;
	float l32 = dl32 / d2;
	float d3 = 0.f - a30 * l30 - dl31 * l31 - dl32 * l32;

	float y3 = x3 - l30 * y0 - l31 * y1 - l32 * y2;
	float z3 = fabsf(d3) > eps ? y3 / d3 : 0.f;

	float lambda = z3;
	float pz = z2 - l32 * lambda;
	float py = z1 - l21 * pz - l31 * lambda;
	float px = z0 - l10 * py - l20 * pz - l30 * lambda;

	p.x = px;
	p.y = py;
	p.z = pz;

	return fabsf(d0) > eps && fabsf(d1) > eps && fabsf(d2) > eps;
}

static void quadric_reduce_attributes(error_quadric& Q, const error_quadric& A, const quadric_gradient* G, size_t attribute_count) {

	Q.diagonal.x += A.diagonal.x * Q.weight;
	Q.diagonal.y += A.diagonal.y * Q.weight;
	Q.diagonal.z += A.diagonal.z * Q.weight;
	Q.off_diagonal.x += A.off_diagonal.x * Q.weight;
	Q.off_diagonal.y += A.off_diagonal.y * Q.weight;
	Q.off_diagonal.z += A.off_diagonal.z * Q.weight;
	Q.linear.x += A.linear.x * Q.weight;
	Q.linear.y += A.linear.y * Q.weight;
	Q.linear.z += A.linear.z * Q.weight;

	float iaw = A.weight == 0 ? 0.f : Q.weight / A.weight;

	for (size_t k = 0; k < attribute_count; ++k) {
		const quadric_gradient& g = G[k];

		Q.diagonal.x -= (g.coefficients.x * g.coefficients.x) * iaw;
		Q.diagonal.y -= (g.coefficients.y * g.coefficients.y) * iaw;
		Q.diagonal.z -= (g.coefficients.z * g.coefficients.z) * iaw;
		Q.off_diagonal.x -= (g.coefficients.x * g.coefficients.y) * iaw;
		Q.off_diagonal.y -= (g.coefficients.x * g.coefficients.z) * iaw;
		Q.off_diagonal.z -= (g.coefficients.y * g.coefficients.z) * iaw;

		Q.linear.x -= (g.coefficients.x * g.coefficients.w) * iaw;
		Q.linear.y -= (g.coefficients.y * g.coefficients.w) * iaw;
		Q.linear.z -= (g.coefficients.z * g.coefficients.w) * iaw;
	}
}

static void fill_face_quadrics(error_quadric* vertex_quadrics, quadric_gradient* volume_gradients, const unsigned int* indices, size_t index_count, const mars::vector3<float>* vertex_positions, const unsigned int* remap) {
	for (size_t i = 0; i < index_count; i += 3) {
		unsigned int i0 = indices[i + 0];
		unsigned int i1 = indices[i + 1];
		unsigned int i2 = indices[i + 2];

		error_quadric Q;
		quadric_from_triangle(Q, vertex_positions[i0], vertex_positions[i1], vertex_positions[i2], 1.f);

		quadric_add(vertex_quadrics[remap[i0]], Q);
		quadric_add(vertex_quadrics[remap[i1]], Q);
		quadric_add(vertex_quadrics[remap[i2]], Q);

		if (volume_gradients) {
			quadric_gradient GV;
			quadric_volume_gradient(GV, vertex_positions[i0], vertex_positions[i1], vertex_positions[i2]);

			quadric_add(volume_gradients[remap[i0]], GV);
			quadric_add(volume_gradients[remap[i1]], GV);
			quadric_add(volume_gradients[remap[i2]], GV);
		}
	}
}

static void fill_vertex_quadrics(error_quadric* vertex_quadrics, const mars::vector3<float>* vertex_positions, size_t vertex_count, const unsigned int* remap, unsigned int options) {

	float factor = (options & mesh_simplify_regularize_flag) ? 1e-1f : 1e-7f;

	for (size_t i = 0; i < vertex_count; ++i) {
		if (remap[i] != i)
			continue;

		const mars::vector3<float>& p = vertex_positions[i];
		float w = vertex_quadrics[i].weight * factor;

		error_quadric Q;
		quadric_from_point(Q, p.x, p.y, p.z, w);

		quadric_add(vertex_quadrics[i], Q);
	}
}

static void fill_edge_quadrics(error_quadric* vertex_quadrics, const unsigned int* indices, size_t index_count, const mars::vector3<float>* vertex_positions, const unsigned int* remap, const unsigned char* vertex_kind, const unsigned int* loop, const unsigned int* loopback) {
	for (size_t i = 0; i < index_count; i += 3) {
		static const int next[4] = {1, 2, 0, 1};

		for (int e = 0; e < 3; ++e) {
			unsigned int i0 = indices[i + e];
			unsigned int i1 = indices[i + next[e]];

			unsigned char k0 = vertex_kind[i0];
			unsigned char k1 = vertex_kind[i1];

			if (k0 != MARS_VERTEX_TYPE_BORDER && k0 != MARS_VERTEX_TYPE_SEAM && k1 != MARS_VERTEX_TYPE_BORDER && k1 != MARS_VERTEX_TYPE_SEAM)
				continue;

			if ((k0 == MARS_VERTEX_TYPE_BORDER || k0 == MARS_VERTEX_TYPE_SEAM) && loop[i0] != i1)
				continue;

			if ((k1 == MARS_VERTEX_TYPE_BORDER || k1 == MARS_VERTEX_TYPE_SEAM) && loopback[i1] != i0)
				continue;

			unsigned int i2 = indices[i + next[e + 1]];

			const float k_edge_weight_seam = 0.5f;
			const float k_edge_weight_border = 10.f;

			float edge_weight = (k0 == MARS_VERTEX_TYPE_BORDER || k1 == MARS_VERTEX_TYPE_BORDER) ? k_edge_weight_border : k_edge_weight_seam;

			error_quadric Q;
			quadric_from_triangle_edge(Q, vertex_positions[i0], vertex_positions[i1], vertex_positions[i2], edge_weight);

			error_quadric QT;
			quadric_from_triangle(QT, vertex_positions[i0], vertex_positions[i1], vertex_positions[i2], edge_weight);

			QT.weight = 0;
			quadric_add(Q, QT);

			quadric_add(vertex_quadrics[remap[i0]], Q);
			quadric_add(vertex_quadrics[remap[i1]], Q);
		}
	}
}

static void fill_attribute_quadrics(error_quadric* attribute_quadrics, quadric_gradient* attribute_gradients, const unsigned int* indices, size_t index_count, const mars::vector3<float>* vertex_positions, const float* vertex_attributes, size_t attribute_count) {
	for (size_t i = 0; i < index_count; i += 3) {
		unsigned int i0 = indices[i + 0];
		unsigned int i1 = indices[i + 1];
		unsigned int i2 = indices[i + 2];

		error_quadric QA;
		quadric_gradient G[k_max_attributes];
		quadric_from_attributes(QA, G, vertex_positions[i0], vertex_positions[i1], vertex_positions[i2], &vertex_attributes[i0 * attribute_count], &vertex_attributes[i1 * attribute_count], &vertex_attributes[i2 * attribute_count], attribute_count);

		quadric_add(attribute_quadrics[i0], QA);
		quadric_add(attribute_quadrics[i1], QA);
		quadric_add(attribute_quadrics[i2], QA);

		quadric_add(&attribute_gradients[i0 * attribute_count], G, attribute_count);
		quadric_add(&attribute_gradients[i1 * attribute_count], G, attribute_count);
		quadric_add(&attribute_gradients[i2 * attribute_count], G, attribute_count);
	}
}

static bool has_triangle_flip(const mars::vector3<float>& a, const mars::vector3<float>& b, const mars::vector3<float>& c, const mars::vector3<float>& d) {
	mars::vector3<float> eb = {b.x - a.x, b.y - a.y, b.z - a.z};
	mars::vector3<float> ec = {c.x - a.x, c.y - a.y, c.z - a.z};
	mars::vector3<float> ed = {d.x - a.x, d.y - a.y, d.z - a.z};

	mars::vector3<float> nbc = {eb.y * ec.z - eb.z * ec.y, eb.z * ec.x - eb.x * ec.z, eb.x * ec.y - eb.y * ec.x};
	mars::vector3<float> nbd = {eb.y * ed.z - eb.z * ed.y, eb.z * ed.x - eb.x * ed.z, eb.x * ed.y - eb.y * ed.x};

	float ndp = nbc.x * nbd.x + nbc.y * nbd.y + nbc.z * nbd.z;
	float abc = nbc.x * nbc.x + nbc.y * nbc.y + nbc.z * nbc.z;
	float abd = nbd.x * nbd.x + nbd.y * nbd.y + nbd.z * nbd.z;

	return ndp <= 0.25f * sqrtf(abc * abd);
}

static bool has_triangle_flips(const vertex_edge_adjacency& adjacency, const mars::vector3<float>* vertex_positions, const unsigned int* collapse_remap, unsigned int i0, unsigned int i1) {
	assert(collapse_remap[i0] == i0);
	assert(collapse_remap[i1] == i1);

	const mars::vector3<float>& v0 = vertex_positions[i0];
	const mars::vector3<float>& v1 = vertex_positions[i1];

	const vertex_edge_adjacency::adjacency_edge* edges = &adjacency.data[adjacency.offsets[i0]];
	size_t count = adjacency.offsets[i0 + 1] - adjacency.offsets[i0];

	for (size_t i = 0; i < count; ++i) {
		unsigned int a = collapse_remap[edges[i].next];
		unsigned int b = collapse_remap[edges[i].prev];

		if (a == i1 || b == i1 || a == b)
			continue;

		if (has_triangle_flip(vertex_positions[a], vertex_positions[b], v0, v1)) {
#if TRACE >= 2
			printf("edge block %d -> %d: flip welded %d %d %d\n", i0, i1, a, i0, b);
#endif

			return true;
		}
	}

	return false;
}

static bool has_triangle_flips(const vertex_edge_adjacency& adjacency, const mars::vector3<float>* vertex_positions, unsigned int i0, const mars::vector3<float>& v1) {
	const mars::vector3<float>& v0 = vertex_positions[i0];

	const vertex_edge_adjacency::adjacency_edge* edges = &adjacency.data[adjacency.offsets[i0]];
	size_t count = adjacency.offsets[i0 + 1] - adjacency.offsets[i0];

	for (size_t i = 0; i < count; ++i) {
		unsigned int a = edges[i].next, b = edges[i].prev;

		if (has_triangle_flip(vertex_positions[a], vertex_positions[b], v0, v1))
			return true;
	}

	return false;
}

static float get_neighborhood_radius(const vertex_edge_adjacency& adjacency, const mars::vector3<float>* vertex_positions, unsigned int i0) {
	const mars::vector3<float>& v0 = vertex_positions[i0];

	const vertex_edge_adjacency::adjacency_edge* edges = &adjacency.data[adjacency.offsets[i0]];
	size_t count = adjacency.offsets[i0 + 1] - adjacency.offsets[i0];

	float result = 0.f;

	for (size_t i = 0; i < count; ++i) {
		unsigned int a = edges[i].next, b = edges[i].prev;

		const mars::vector3<float>& va = vertex_positions[a];
		const mars::vector3<float>& vb = vertex_positions[b];

		float da = (va.x - v0.x) * (va.x - v0.x) + (va.y - v0.y) * (va.y - v0.y) + (va.z - v0.z) * (va.z - v0.z);
		float db = (vb.x - v0.x) * (vb.x - v0.x) + (vb.y - v0.y) * (vb.y - v0.y) + (vb.z - v0.z) * (vb.z - v0.z);

		result = result < da ? da : result;
		result = result < db ? db : result;
	}

	return sqrtf(result);
}

static unsigned int get_complex_target(unsigned int v, unsigned int target, const unsigned int* remap, const unsigned int* loop, const unsigned int* loopback) {
	unsigned int r = remap[target];

	if (loop[v] != ~0u && remap[loop[v]] == r)
		return loop[v];
	else if (loopback[v] != ~0u && remap[loopback[v]] == r)
		return loopback[v];
	else
		return target;
}

static size_t bound_edge_collapses(const vertex_edge_adjacency& adjacency, size_t vertex_count, size_t index_count, unsigned char* vertex_kind) {
	size_t dual_count = 0;

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned char k = vertex_kind[i];
		unsigned int e = adjacency.offsets[i + 1] - adjacency.offsets[i];

		dual_count += (k == MARS_VERTEX_TYPE_MANIFOLD || k == MARS_VERTEX_TYPE_SEAM) ? e : 0;
	}

	assert(dual_count <= index_count);

	return (index_count - dual_count / 2) + 3;
}

static size_t pick_edge_collapses(edge_collapse* collapses, size_t collapse_capacity, const unsigned int* indices, size_t index_count, const unsigned int* remap, const unsigned char* vertex_kind, const unsigned int* loop, const unsigned int* loopback) {
	size_t collapse_count = 0;

	for (size_t i = 0; i < index_count; i += 3) {
		static const int next[3] = {1, 2, 0};

		if (collapse_count + 3 > collapse_capacity)
			break;

		for (int e = 0; e < 3; ++e) {
			unsigned int i0 = indices[i + e];
			unsigned int i1 = indices[i + next[e]];

			if (remap[i0] == remap[i1])
				continue;

			unsigned char k0 = vertex_kind[i0];
			unsigned char k1 = vertex_kind[i1];

			if (!(k_can_collapse[k0][k1] | k_can_collapse[k1][k0]))
				continue;

			if (k_has_opposite[k0][k1] && remap[i1] > remap[i0])
				continue;

			if ((k0 == MARS_VERTEX_TYPE_BORDER || k0 == MARS_VERTEX_TYPE_SEAM) && k1 != MARS_VERTEX_TYPE_MANIFOLD && loop[i0] != i1)
				continue;
			if ((k1 == MARS_VERTEX_TYPE_BORDER || k1 == MARS_VERTEX_TYPE_SEAM) && k0 != MARS_VERTEX_TYPE_MANIFOLD && loopback[i1] != i0)
				continue;

			if (k_can_collapse[k0][k1] & k_can_collapse[k1][k0]) {
				edge_collapse c = {i0, i1, true};
				collapses[collapse_count++] = c;
			} else {

				unsigned int e0 = k_can_collapse[k0][k1] ? i0 : i1;
				unsigned int e1 = k_can_collapse[k0][k1] ? i1 : i0;

				edge_collapse c = {e0, e1, false};
				collapses[collapse_count++] = c;
			}
		}
	}

	return collapse_count;
}

static void rank_edge_collapses(edge_collapse* collapses, size_t collapse_count, const mars::vector3<float>* vertex_positions, const float* vertex_attributes, const error_quadric* vertex_quadrics, const error_quadric* attribute_quadrics, const quadric_gradient* attribute_gradients, size_t attribute_count, const unsigned int* remap, const unsigned int* wedge, const unsigned char* vertex_kind, const unsigned int* loop, const unsigned int* loopback) {
	for (size_t i = 0; i < collapse_count; ++i) {
		edge_collapse& c = collapses[i];

		unsigned int i0 = c.v0;
		unsigned int i1 = c.v1;
		bool bidi = c.bidirectional;

		float ei = quadric_error(vertex_quadrics[remap[i0]], vertex_positions[i1]);
		float ej = bidi ? quadric_error(vertex_quadrics[remap[i1]], vertex_positions[i0]) : FLT_MAX;

#if TRACE >= 3
		float di = ei, dj = ej;
#endif

		if (attribute_count) {
			ei += quadric_error(attribute_quadrics[i0], &attribute_gradients[i0 * attribute_count], attribute_count, vertex_positions[i1], &vertex_attributes[i1 * attribute_count]);
			ej += bidi ? quadric_error(attribute_quadrics[i1], &attribute_gradients[i1 * attribute_count], attribute_count, vertex_positions[i0], &vertex_attributes[i0 * attribute_count]) : 0;

			if (vertex_kind[i0] == MARS_VERTEX_TYPE_SEAM) {

				unsigned int s0 = wedge[i0];
				unsigned int s1 = loop[i0] == i1 ? loopback[s0] : loop[s0];

				assert(wedge[s0] == i0);
				assert(s1 != ~0u && remap[s1] == remap[i1]);

				s1 = (s1 != ~0u) ? s1 : wedge[i1];

				ei += quadric_error(attribute_quadrics[s0], &attribute_gradients[s0 * attribute_count], attribute_count, vertex_positions[s1], &vertex_attributes[s1 * attribute_count]);
				ej += bidi ? quadric_error(attribute_quadrics[s1], &attribute_gradients[s1 * attribute_count], attribute_count, vertex_positions[s0], &vertex_attributes[s0 * attribute_count]) : 0;
			} else {

				if (vertex_kind[i0] == MARS_VERTEX_TYPE_COMPLEX)
					for (unsigned int v = wedge[i0]; v != i0; v = wedge[v]) {
						unsigned int t = get_complex_target(v, i1, remap, loop, loopback);

						ei += quadric_error(attribute_quadrics[v], &attribute_gradients[v * attribute_count], attribute_count, vertex_positions[t], &vertex_attributes[t * attribute_count]);
					}

				if (vertex_kind[i1] == MARS_VERTEX_TYPE_COMPLEX && bidi)
					for (unsigned int v = wedge[i1]; v != i1; v = wedge[v]) {
						unsigned int t = get_complex_target(v, i0, remap, loop, loopback);

						ej += quadric_error(attribute_quadrics[v], &attribute_gradients[v * attribute_count], attribute_count, vertex_positions[t], &vertex_attributes[t * attribute_count]);
					}
			}
		}

		bool rev = bidi & (ej < ei);

		c.v0 = rev ? i1 : i0;
		c.v1 = rev ? i0 : i1;
		c.error = ej < ei ? ej : ei;

#if TRACE >= 3
		if (bidi)
			printf("edge eval %d -> %d: error %f (pos %f, attr %f); reverse %f (pos %f, attr %f)\n",
			       rev ? i1 : i0, rev ? i0 : i1,
			       sqrtf(rev ? ej : ei), sqrtf(rev ? dj : di), sqrtf(rev ? ej - dj : ei - di),
			       sqrtf(rev ? ei : ej), sqrtf(rev ? di : dj), sqrtf(rev ? ei - di : ej - dj));
		else
			printf("edge eval %d -> %d: error %f (pos %f, attr %f)\n", i0, i1, sqrtf(c.error), sqrtf(di), sqrtf(ei - di));
#endif
	}
}

static void sort_edge_collapses(unsigned int* sort_order, const edge_collapse* collapses, size_t collapse_count) {

	const unsigned int sort_bits = 12;
	const unsigned int sort_bins = 2048 + 512;

	unsigned int histogram[sort_bins];
	memset(histogram, 0, sizeof(histogram));

	for (size_t i = 0; i < collapse_count; ++i) {

		unsigned int error = sortable_error_bits(collapses[i].error);
		unsigned int key = (error << 1) >> (32 - sort_bits);
		key = key < sort_bins ? key : sort_bins - 1;

		histogram[key]++;
	}

	size_t histogram_sum = 0;

	for (size_t i = 0; i < sort_bins; ++i) {
		size_t count = histogram[i];
		histogram[i] = unsigned(histogram_sum);
		histogram_sum += count;
	}

	assert(histogram_sum == collapse_count);

	for (size_t i = 0; i < collapse_count; ++i) {

		unsigned int error = sortable_error_bits(collapses[i].error);
		unsigned int key = (error << 1) >> (32 - sort_bits);
		key = key < sort_bins ? key : sort_bins - 1;

		sort_order[histogram[key]++] = unsigned(i);
	}
}

static size_t perform_edge_collapses(unsigned int* collapse_remap, unsigned char* collapse_locked, const edge_collapse* collapses, size_t collapse_count, const unsigned int* collapse_order, const unsigned int* remap, const unsigned int* wedge, const unsigned char* vertex_kind, const unsigned int* loop, const unsigned int* loopback, const mars::vector3<float>* vertex_positions, const vertex_edge_adjacency& adjacency, size_t triangle_collapse_goal, float error_limit, float& result_error) {
	size_t edge_collapses = 0;
	size_t triangle_collapses = 0;

	size_t edge_collapse_goal = triangle_collapse_goal / 2;

#if TRACE
	size_t stats[7] = {};
#endif

	for (size_t i = 0; i < collapse_count; ++i) {
		const edge_collapse& c = collapses[collapse_order[i]];

		TRACESTATS(0);

		if (c.error > error_limit) {
			TRACESTATS(4);
			break;
		}

		if (triangle_collapses >= triangle_collapse_goal) {
			TRACESTATS(5);
			break;
		}

		float error_goal = edge_collapse_goal < collapse_count ? 1.5f * collapses[collapse_order[edge_collapse_goal]].error : FLT_MAX;

		if (c.error > error_goal && c.error > result_error && triangle_collapses > triangle_collapse_goal / 6) {
			TRACESTATS(6);
			break;
		}

		unsigned int i0 = c.v0;
		unsigned int i1 = c.v1;

		unsigned int r0 = remap[i0];
		unsigned int r1 = remap[i1];

		unsigned char kind = vertex_kind[i0];

		if (collapse_locked[r0] | collapse_locked[r1]) {
			TRACESTATS(1);
			continue;
		}

		if (has_triangle_flips(adjacency, vertex_positions, collapse_remap, r0, r1)) {

			edge_collapse_goal++;

			TRACESTATS(2);
			continue;
		}

#if TRACE >= 2
		printf("edge commit %d -> %d: kind %d->%d, error %f\n", i0, i1, vertex_kind[i0], vertex_kind[i1], sqrtf(c.error));
#endif

		assert(collapse_remap[r0] == r0);
		assert(collapse_remap[r1] == r1);

		if (kind == MARS_VERTEX_TYPE_COMPLEX) {

			unsigned int v = i0;

			do {
				unsigned int t = get_complex_target(v, i1, remap, loop, loopback);

				collapse_remap[v] = t;
				v = wedge[v];
			} while (v != i0);
		} else if (kind == MARS_VERTEX_TYPE_SEAM) {

			unsigned int s0 = wedge[i0];
			unsigned int s1 = loop[i0] == i1 ? loopback[s0] : loop[s0];
			assert(wedge[s0] == i0);
			assert(s1 != ~0u && remap[s1] == r1);

			assert(kind != vertex_kind[i1] || s1 == wedge[i1]);
			assert(loop[i0] == i1 || loopback[i0] == i1);
			assert(loop[s0] == s1 || loopback[s0] == s1);

			s1 = (s1 != ~0u) ? s1 : wedge[i1];

			collapse_remap[i0] = i1;
			collapse_remap[s0] = s1;
		} else {
			assert(wedge[i0] == i0);

			collapse_remap[i0] = i1;
		}

		collapse_locked[r0] = 1;
		collapse_locked[r1] = 1;

		triangle_collapses += (kind == MARS_VERTEX_TYPE_BORDER) ? 1 : 2;
		edge_collapses++;

		result_error = result_error < c.error ? c.error : result_error;
	}

#if TRACE
	float error_goal_last = edge_collapse_goal < collapse_count ? 1.5f * collapses[collapse_order[edge_collapse_goal]].error : FLT_MAX;
	float error_goal_limit = error_goal_last < error_limit ? error_goal_last : error_limit;

	printf("removed %d triangles, error %e (goal %e); evaluated %d/%d collapses (done %d, skipped %d, invalid %d); %s\n",
	       int(triangle_collapses), sqrtf(result_error), sqrtf(error_goal_limit),
	       int(stats[0]), int(collapse_count), int(edge_collapses), int(stats[1]), int(stats[2]),
	       stats[4] ? "error limit" : (stats[5] ? "count limit" : (stats[6] ? "error goal" : "out of collapses")));
#endif

	return edge_collapses;
}

static void update_quadrics(const unsigned int* collapse_remap, size_t vertex_count, error_quadric* vertex_quadrics, quadric_gradient* volume_gradients, error_quadric* attribute_quadrics, quadric_gradient* attribute_gradients, size_t attribute_count, const mars::vector3<float>* vertex_positions, const unsigned int* remap, float& vertex_error) {
	for (size_t i = 0; i < vertex_count; ++i) {
		if (collapse_remap[i] == i)
			continue;

		unsigned int i0 = unsigned(i);
		unsigned int i1 = collapse_remap[i];

		unsigned int r0 = remap[i0];
		unsigned int r1 = remap[i1];

		if (i0 == r0) {
			quadric_add(vertex_quadrics[r1], vertex_quadrics[r0]);

			if (volume_gradients)
				quadric_add(volume_gradients[r1], volume_gradients[r0]);
		}

		if (attribute_count) {
			quadric_add(attribute_quadrics[i1], attribute_quadrics[i0]);
			quadric_add(&attribute_gradients[i1 * attribute_count], &attribute_gradients[i0 * attribute_count], attribute_count);

			if (i0 == r0) {

				float derr = quadric_error(vertex_quadrics[r0], vertex_positions[r1]);
				vertex_error = vertex_error < derr ? derr : vertex_error;
			}
		}
	}
}

static void solve_positions(mars::vector3<float>* vertex_positions, size_t vertex_count, const error_quadric* vertex_quadrics, const quadric_gradient* volume_gradients, const error_quadric* attribute_quadrics, const quadric_gradient* attribute_gradients, size_t attribute_count, const unsigned int* remap, const unsigned int* wedge, const vertex_edge_adjacency& adjacency, const unsigned char* vertex_kind, const unsigned char* vertex_update) {
#if TRACE
	size_t stats[6] = {};
#endif

	for (size_t i = 0; i < vertex_count; ++i) {
		if (!vertex_update[i])
			continue;

		if (vertex_kind[i] == MARS_VERTEX_TYPE_LOCKED || vertex_kind[i] == MARS_VERTEX_TYPE_SEAM || vertex_kind[i] == MARS_VERTEX_TYPE_BORDER)
			continue;

		if (remap[i] != i) {
			vertex_positions[i] = vertex_positions[remap[i]];
			continue;
		}

		TRACESTATS(0);

		const mars::vector3<float>& vp = vertex_positions[i];

		error_quadric Q = vertex_quadrics[i];
		quadric_gradient GV = {};

		error_quadric R;
		quadric_from_point(R, vp.x, vp.y, vp.z, Q.weight * 1e-4f);
		quadric_add(Q, R);

		if (attribute_count) {

			unsigned int v = unsigned(i);
			do {
				quadric_reduce_attributes(Q, attribute_quadrics[v], &attribute_gradients[v * attribute_count], attribute_count);
				v = wedge[v];
			} while (v != i);

			if (volume_gradients)
				GV = volume_gradients[i];
		}

		mars::vector3<float> p;
		if (!quadric_solve(p, Q, GV)) {
			TRACESTATS(2);
			continue;
		}

		float nr = get_neighborhood_radius(adjacency, vertex_positions, unsigned(i));
		float dp = (p.x - vp.x) * (p.x - vp.x) + (p.y - vp.y) * (p.y - vp.y) + (p.z - vp.z) * (p.z - vp.z);

		if (dp > nr * nr) {
			TRACESTATS(3);
			continue;
		}

		if (has_triangle_flips(adjacency, vertex_positions, unsigned(i), p)) {
			TRACESTATS(4);
			continue;
		}

		if (quadric_error(vertex_quadrics[i], p) > quadric_error(vertex_quadrics[i], vp) * 1.5f + 1e-6f) {
			TRACESTATS(5);
			continue;
		}

		TRACESTATS(1);
		vertex_positions[i] = p;
	}

#if TRACE
	printf("updated %d/%d positions; failed solve %d bounds %d flip %d error %d\n", int(stats[1]), int(stats[0]), int(stats[2]), int(stats[3]), int(stats[4]), int(stats[5]));
#endif
}

static void solve_attributes(mars::vector3<float>* vertex_positions, float* vertex_attributes, size_t vertex_count, const error_quadric* attribute_quadrics, const quadric_gradient* attribute_gradients, size_t attribute_count, const unsigned int* remap, const unsigned int* wedge, const unsigned char* vertex_kind, const unsigned char* vertex_update) {
	for (size_t i = 0; i < vertex_count; ++i) {
		if (!vertex_update[i])
			continue;

		if (remap[i] != i)
			continue;

		for (size_t k = 0; k < attribute_count; ++k) {
			unsigned int shared = ~0u;

			if (vertex_kind[i] == MARS_VERTEX_TYPE_COMPLEX) {
				shared = unsigned(i);

				for (unsigned int v = wedge[i]; v != i; v = wedge[v])
					if (vertex_attributes[v * attribute_count + k] != vertex_attributes[i * attribute_count + k])
						shared = ~0u;
					else if (shared != ~0u && attribute_quadrics[v].weight > attribute_quadrics[shared].weight)
						shared = v;
			}

			unsigned int v = unsigned(i);
			do {
				unsigned int r = (shared == ~0u) ? v : shared;

				const mars::vector3<float>& p = vertex_positions[i];
				const error_quadric& A = attribute_quadrics[r];
				const quadric_gradient& G = attribute_gradients[r * attribute_count + k];

				float iw = A.weight == 0 ? 0.f : 1.f / A.weight;
				float av = (G.coefficients.x * p.x + G.coefficients.y * p.y + G.coefficients.z * p.z + G.coefficients.w) * iw;

				vertex_attributes[v * attribute_count + k] = av;
				v = wedge[v];
			} while (v != i);
		}
	}
}

static size_t remap_index_buffer(unsigned int* indices, size_t index_count, const unsigned int* collapse_remap, const unsigned int* remap) {
	size_t write = 0;

	for (size_t i = 0; i < index_count; i += 3) {
		unsigned int v0 = collapse_remap[indices[i + 0]];
		unsigned int v1 = collapse_remap[indices[i + 1]];
		unsigned int v2 = collapse_remap[indices[i + 2]];

		assert(collapse_remap[v0] == v0);
		assert(collapse_remap[v1] == v1);
		assert(collapse_remap[v2] == v2);

		unsigned int r0 = remap[v0];
		unsigned int r1 = remap[v1];
		unsigned int r2 = remap[v2];

		if (r0 != r1 && r0 != r2 && r1 != r2) {
			indices[write + 0] = v0;
			indices[write + 1] = v1;
			indices[write + 2] = v2;
			write += 3;
		}
	}

	return write;
}

static void remap_edge_loops(unsigned int* loop, size_t vertex_count, const unsigned int* collapse_remap) {
	for (size_t i = 0; i < vertex_count; ++i) {

		if (loop[i] != ~0u) {
			unsigned int l = loop[i];
			unsigned int r = collapse_remap[l];

			if (i == r)
				loop[i] = (loop[l] != ~0u) ? collapse_remap[loop[l]] : ~0u;
			else
				loop[i] = r;
		}
	}
}

static unsigned int follow(unsigned int* parents, unsigned int index) {
	while (index != parents[index]) {
		unsigned int parent = parents[index];
		parents[index] = parents[parent];
		index = parent;
	}

	return index;
}

static size_t build_components(unsigned int* components, size_t vertex_count, const unsigned int* indices, size_t index_count, const unsigned int* remap) {
	for (size_t i = 0; i < vertex_count; ++i)
		components[i] = unsigned(i);

	for (size_t i = 0; i < index_count; i += 3) {
		static const int next[4] = {1, 2, 0, 1};

		for (int e = 0; e < 3; ++e) {
			unsigned int i0 = indices[i + e];
			unsigned int i1 = indices[i + next[e]];

			unsigned int r0 = remap[i0];
			unsigned int r1 = remap[i1];

			r0 = follow(components, r0);
			r1 = follow(components, r1);

			if (r0 != r1)
				components[r0 < r1 ? r1 : r0] = r0 < r1 ? r0 : r1;
		}
	}

	for (size_t i = 0; i < vertex_count; ++i)
		if (remap[i] == i)
			components[i] = follow(components, unsigned(i));

	unsigned int next_component = 0;

	for (size_t i = 0; i < vertex_count; ++i) {
		if (remap[i] == i) {
			unsigned int root = components[i];
			assert(root <= i);
			components[i] = (root == i) ? next_component++ : components[root];
		} else {
			assert(remap[i] < i);
			components[i] = components[remap[i]];
		}
	}

	return next_component;
}

static void measure_components(float* component_errors, size_t component_count, const unsigned int* components, const mars::vector3<float>* vertex_positions, size_t vertex_count) {
	memset(component_errors, 0, component_count * 4 * sizeof(float));

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int c = components[i];
		assert(components[i] < component_count);

		mars::vector3<float> v = vertex_positions[i];

		component_errors[c * 4 + 0] += v.x;
		component_errors[c * 4 + 1] += v.y;
		component_errors[c * 4 + 2] += v.z;
		component_errors[c * 4 + 3] += 1;
	}

	for (size_t i = 0; i < component_count; ++i) {
		float w = component_errors[i * 4 + 3];
		float iw = w == 0.f ? 0.f : 1.f / w;

		component_errors[i * 4 + 0] *= iw;
		component_errors[i * 4 + 1] *= iw;
		component_errors[i * 4 + 2] *= iw;
		component_errors[i * 4 + 3] = 0;
	}

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int c = components[i];

		float dx = vertex_positions[i].x - component_errors[c * 4 + 0];
		float dy = vertex_positions[i].y - component_errors[c * 4 + 1];
		float dz = vertex_positions[i].z - component_errors[c * 4 + 2];
		float r = dx * dx + dy * dy + dz * dz;

		component_errors[c * 4 + 3] = component_errors[c * 4 + 3] < r ? r : component_errors[c * 4 + 3];
	}

	for (size_t i = 0; i < component_count; ++i) {
#if TRACE >= 2
		printf("component %d: center %f %f %f, error %e\n", int(i),
		       component_errors[i * 4 + 0], component_errors[i * 4 + 1], component_errors[i * 4 + 2], sqrtf(component_errors[i * 4 + 3]));
#endif

		component_errors[i] = component_errors[i * 4 + 3];
	}
}

static size_t prune_components(unsigned int* indices, size_t index_count, const unsigned int* components, const float* component_errors, size_t component_count, float error_cutoff, float& nexterror) {
	(void)component_count;

	size_t write = 0;
	float min_error = FLT_MAX;

	for (size_t i = 0; i < index_count; i += 3) {
		unsigned int v0 = indices[i + 0], v1 = indices[i + 1], v2 = indices[i + 2];
		unsigned int c = components[v0];
		assert(c == components[v1] && c == components[v2]);

		if (component_errors[c] > error_cutoff) {
			min_error = min_error > component_errors[c] ? component_errors[c] : min_error;

			indices[write + 0] = v0;
			indices[write + 1] = v1;
			indices[write + 2] = v2;
			write += 3;
		}
	}

#if TRACE
	size_t pruned_components = 0;
	for (size_t i = 0; i < component_count; ++i)
		pruned_components += (component_errors[i] >= nexterror && component_errors[i] <= error_cutoff);

	printf("pruned %d triangles in %d components (goal %e); next %e\n", int((index_count - write) / 3), int(pruned_components), sqrtf(error_cutoff), min_error < FLT_MAX ? sqrtf(min_error) : min_error * 2);
#endif

	nexterror = min_error;
	return write;
}

size_t vertex_cell_hasher::hash(unsigned int i) const {
	unsigned int h = vertex_ids[i];

	h ^= h >> 13;
	h *= 0x5bd1e995;
	h ^= h >> 15;
	return h;
}

bool vertex_cell_hasher::equal(unsigned int lhs, unsigned int rhs) const {
	return vertex_ids[lhs] == vertex_ids[rhs];
}

size_t integer_id_hasher::hash(unsigned int id) const {
	unsigned int h = id;

	h ^= h >> 13;
	h *= 0x5bd1e995;
	h ^= h >> 15;
	return h;
}

bool integer_id_hasher::equal(unsigned int lhs, unsigned int rhs) const {
	return lhs == rhs;
}

size_t triangle_index_triplet_hasher::hash(unsigned int i) const {
	const unsigned int* tri = indices + i * 3;

	return (tri[0] * 73856093) ^ (tri[1] * 19349663) ^ (tri[2] * 83492791);
}

bool triangle_index_triplet_hasher::equal(unsigned int lhs, unsigned int rhs) const {
	const unsigned int* lt = indices + lhs * 3;
	const unsigned int* rt = indices + rhs * 3;

	return lt[0] == rt[0] && lt[1] == rt[1] && lt[2] == rt[2];
}

static void compute_vertex_ids(unsigned int* vertex_ids, const mars::vector3<float>* vertex_positions, const unsigned char* vertex_lock, size_t vertex_count, int grid_size) {
	assert(grid_size >= 1 && grid_size <= 1024);
	float cell_scale = float(grid_size - 1);

	for (size_t i = 0; i < vertex_count; ++i) {
		const mars::vector3<float>& v = vertex_positions[i];

		int xi = int(v.x * cell_scale + 0.5f);
		int yi = int(v.y * cell_scale + 0.5f);
		int zi = int(v.z * cell_scale + 0.5f);

		if (vertex_lock && (vertex_lock[i] & mesh_simplify_vertex_lock))
			vertex_ids[i] = (1 << 30) | unsigned(i);
		else
			vertex_ids[i] = (xi << 20) | (yi << 10) | zi;
	}
}

static size_t count_triangles(const unsigned int* vertex_ids, const unsigned int* indices, size_t index_count) {
	size_t result = 0;

	for (size_t i = 0; i < index_count; i += 3) {
		unsigned int id0 = vertex_ids[indices[i + 0]];
		unsigned int id1 = vertex_ids[indices[i + 1]];
		unsigned int id2 = vertex_ids[indices[i + 2]];

		result += (id0 != id1) & (id0 != id2) & (id1 != id2);
	}

	return result;
}

static size_t fill_vertex_cells(unsigned int* table, size_t table_size, unsigned int* vertex_cells, const unsigned int* vertex_ids, size_t vertex_count) {
	vertex_cell_hasher hasher = {vertex_ids};

	memset(table, -1, table_size * sizeof(unsigned int));

	size_t result = 0;

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int* entry = hash_lookup2(table, table_size, hasher, unsigned(i), ~0u);

		if (*entry == ~0u) {
			*entry = unsigned(i);
			vertex_cells[i] = unsigned(result++);
		} else {
			vertex_cells[i] = vertex_cells[*entry];
		}
	}

	return result;
}

static size_t count_vertex_cells(unsigned int* table, size_t table_size, const unsigned int* vertex_ids, size_t vertex_count) {
	integer_id_hasher hasher;

	memset(table, -1, table_size * sizeof(unsigned int));

	size_t result = 0;

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int id = vertex_ids[i];
		unsigned int* entry = hash_lookup2(table, table_size, hasher, id, ~0u);

		result += (*entry == ~0u);
		*entry = id;
	}

	return result;
}

static void fill_cell_quadrics(error_quadric* cell_quadrics, const unsigned int* indices, size_t index_count, const mars::vector3<float>* vertex_positions, const unsigned int* vertex_cells) {
	for (size_t i = 0; i < index_count; i += 3) {
		unsigned int i0 = indices[i + 0];
		unsigned int i1 = indices[i + 1];
		unsigned int i2 = indices[i + 2];

		unsigned int c0 = vertex_cells[i0];
		unsigned int c1 = vertex_cells[i1];
		unsigned int c2 = vertex_cells[i2];

		int single_cell = (c0 == c1) & (c0 == c2);

		error_quadric Q;
		quadric_from_triangle(Q, vertex_positions[i0], vertex_positions[i1], vertex_positions[i2], single_cell ? 3.f : 1.f);

		if (single_cell) {
			quadric_add(cell_quadrics[c0], Q);
		} else {
			quadric_add(cell_quadrics[c0], Q);
			quadric_add(cell_quadrics[c1], Q);
			quadric_add(cell_quadrics[c2], Q);
		}
	}
}

static void fill_cell_reservoirs(point_reservoir* cell_reservoirs, size_t cell_count, const mars::vector3<float>* vertex_positions, const float* vertex_colors, size_t vertex_colors_stride, size_t vertex_count, const unsigned int* vertex_cells) {
	static const float dummy_color[] = {0.f, 0.f, 0.f};

	size_t vertex_colors_stride_float = vertex_colors_stride / sizeof(float);

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int cell = vertex_cells[i];
		const mars::vector3<float>& v = vertex_positions[i];
		point_reservoir& r = cell_reservoirs[cell];

		const float* color = vertex_colors ? &vertex_colors[i * vertex_colors_stride_float] : dummy_color;

		r.position.x += v.x;
		r.position.y += v.y;
		r.position.z += v.z;
		r.color.x += color[0];
		r.color.y += color[1];
		r.color.z += color[2];
		r.weight += 1.f;
	}

	for (size_t i = 0; i < cell_count; ++i) {
		point_reservoir& r = cell_reservoirs[i];

		float iw = r.weight == 0.f ? 0.f : 1.f / r.weight;

		r.position.x *= iw;
		r.position.y *= iw;
		r.position.z *= iw;
		r.color.x *= iw;
		r.color.y *= iw;
		r.color.z *= iw;
	}
}

static void fill_cell_remap(unsigned int* cell_remap, float* cell_errors, size_t cell_count, const unsigned int* vertex_cells, const error_quadric* cell_quadrics, const mars::vector3<float>* vertex_positions, size_t vertex_count) {
	memset(cell_remap, -1, cell_count * sizeof(unsigned int));

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int cell = vertex_cells[i];
		float error = quadric_error(cell_quadrics[cell], vertex_positions[i]);

		if (cell_remap[cell] == ~0u || cell_errors[cell] > error) {
			cell_remap[cell] = unsigned(i);
			cell_errors[cell] = error;
		}
	}
}

static void fill_cell_remap(unsigned int* cell_remap, float* cell_errors, size_t cell_count, const unsigned int* vertex_cells, const point_reservoir* cell_reservoirs, const mars::vector3<float>* vertex_positions, const float* vertex_colors, size_t vertex_colors_stride, float color_weight, size_t vertex_count) {
	static const float dummy_color[] = {0.f, 0.f, 0.f};

	size_t vertex_colors_stride_float = vertex_colors_stride / sizeof(float);

	memset(cell_remap, -1, cell_count * sizeof(unsigned int));

	for (size_t i = 0; i < vertex_count; ++i) {
		unsigned int cell = vertex_cells[i];
		const mars::vector3<float>& v = vertex_positions[i];
		const point_reservoir& r = cell_reservoirs[cell];

		const float* color = vertex_colors ? &vertex_colors[i * vertex_colors_stride_float] : dummy_color;

		float pos_error = (v.x - r.position.x) * (v.x - r.position.x) + (v.y - r.position.y) * (v.y - r.position.y) + (v.z - r.position.z) * (v.z - r.position.z);
		float col_error = (color[0] - r.color.x) * (color[0] - r.color.x) + (color[1] - r.color.y) * (color[1] - r.color.y) + (color[2] - r.color.z) * (color[2] - r.color.z);
		float error = pos_error + color_weight * col_error;

		if (cell_remap[cell] == ~0u || cell_errors[cell] > error) {
			cell_remap[cell] = unsigned(i);
			cell_errors[cell] = error;
		}
	}
}

static size_t filter_triangles(unsigned int* destination, unsigned int* tritable, size_t tritable_size, const unsigned int* indices, size_t index_count, const unsigned int* vertex_cells, const unsigned int* cell_remap) {
	triangle_index_triplet_hasher hasher = {destination};

	memset(tritable, -1, tritable_size * sizeof(unsigned int));

	size_t result = 0;

	for (size_t i = 0; i < index_count; i += 3) {
		unsigned int c0 = vertex_cells[indices[i + 0]];
		unsigned int c1 = vertex_cells[indices[i + 1]];
		unsigned int c2 = vertex_cells[indices[i + 2]];

		if (c0 != c1 && c0 != c2 && c1 != c2) {
			unsigned int a = cell_remap[c0];
			unsigned int b = cell_remap[c1];
			unsigned int c = cell_remap[c2];

			if (b < a && b < c) {
				unsigned int t = a;
				a = b, b = c, c = t;
			} else if (c < a && c < b) {
				unsigned int t = c;
				c = b, b = a, a = t;
			}

			destination[result * 3 + 0] = a;
			destination[result * 3 + 1] = b;
			destination[result * 3 + 2] = c;

			unsigned int* entry = hash_lookup2(tritable, tritable_size, hasher, unsigned(result), ~0u);

			if (*entry == ~0u)
				*entry = unsigned(result++);
		}
	}

	return result * 3;
}

static float interpolate(float y, float x0, float y0, float x1, float y1, float x2, float y2) {

	float num = (y1 - y) * (x1 - x2) * (x1 - x0) * (y2 - y0);
	float den = (y2 - y) * (x1 - x2) * (y0 - y1) + (y0 - y) * (x1 - x0) * (y1 - y2);
	return x1 + (den == 0.f ? 0.f : num / den);
}

} // namespace core

enum {
	mesh_simplify_internal_solve = 1 << 29,
	mesh_simplify_internal_debug = 1 << 30
};

size_t mesh_simplify_edge(unsigned int* destination, const unsigned int* indices, size_t index_count, const float* vertex_positions_data, size_t vertex_count, size_t vertex_positions_stride, const float* vertex_attributes_data, size_t vertex_attributes_stride, const float* attribute_weights, size_t attribute_count, const unsigned char* vertex_lock, size_t target_index_count, float target_error, unsigned int options, float* out_result_error) {
	using namespace mars::mesh::core;

	assert(index_count % 3 == 0);
	assert(vertex_positions_stride >= 12 && vertex_positions_stride <= 256);
	assert(vertex_positions_stride % sizeof(float) == 0);
	assert(target_index_count <= index_count);
	assert(target_error >= 0);
	assert((options & ~(mesh_simplify_lock_border_flag | mesh_simplify_sparse_flag | mesh_simplify_error_absolute_flag | mesh_simplify_prune_flag | mesh_simplify_regularize_flag | mesh_simplify_permissive_flag | mesh_simplify_internal_solve | mesh_simplify_internal_debug)) == 0);
	assert(vertex_attributes_stride >= attribute_count * sizeof(float) && vertex_attributes_stride <= 256);
	assert(vertex_attributes_stride % sizeof(float) == 0);
	assert(attribute_count <= k_max_attributes);
	for (size_t i = 0; i < attribute_count; ++i)
		assert(attribute_weights[i] >= 0);

	array_stack_heap_allocator<24> allocator;

	unsigned int* result = destination;
	if (result != indices)
		memcpy(result, indices, index_count * sizeof(unsigned int));

	unsigned int* sparse_remap = NULL;
	if (options & mesh_simplify_sparse_flag)
		sparse_remap = build_sparse_remap(result, index_count, vertex_count, &vertex_count, allocator);

	vertex_edge_adjacency adjacency = {};
	prepare_edge_adjacency(adjacency, index_count, vertex_count, allocator);
	update_edge_adjacency(adjacency, result, index_count, vertex_count, NULL);

	unsigned int* remap = allocator.allocate<unsigned int>(vertex_count);
	unsigned int* wedge = allocator.allocate<unsigned int>(vertex_count);
	build_position_remap(remap, wedge, vertex_positions_data, vertex_count, vertex_positions_stride, sparse_remap, allocator);

	unsigned char* vertex_kind = allocator.allocate<unsigned char>(vertex_count);
	unsigned int* loop = allocator.allocate<unsigned int>(vertex_count);
	unsigned int* loopback = allocator.allocate<unsigned int>(vertex_count);
	classify_vertices(vertex_kind, loop, loopback, vertex_count, adjacency, remap, wedge, vertex_lock, sparse_remap, options);

#if TRACE
	size_t unique_positions = 0;
	for (size_t i = 0; i < vertex_count; ++i)
		unique_positions += remap[i] == i;

	printf("position remap: %d vertices => %d positions\n", int(vertex_count), int(unique_positions));

	size_t kinds[MARS_VERTEX_TYPE_COUNT] = {};
	for (size_t i = 0; i < vertex_count; ++i)
		kinds[vertex_kind[i]] += remap[i] == i;

	printf("kinds: manifold %d, border %d, seam %d, complex %d, locked %d\n",
	       int(kinds[MARS_VERTEX_TYPE_MANIFOLD]), int(kinds[MARS_VERTEX_TYPE_BORDER]), int(kinds[MARS_VERTEX_TYPE_SEAM]), int(kinds[MARS_VERTEX_TYPE_COMPLEX]), int(kinds[MARS_VERTEX_TYPE_LOCKED]));
#endif

	mars::vector3<float>* vertex_positions = allocator.allocate<mars::vector3<float>>(vertex_count);
	float vertex_offset[3] = {};
	float vertex_scale = rescale_positions(vertex_positions, vertex_positions_data, vertex_count, vertex_positions_stride, sparse_remap, vertex_offset);

	float* vertex_attributes = NULL;
	unsigned int attribute_remap[k_max_attributes];

	if (attribute_count) {

		size_t attributes_used = 0;
		for (size_t i = 0; i < attribute_count; ++i)
			if (attribute_weights[i] > 0)
				attribute_remap[attributes_used++] = unsigned(i);

		attribute_count = attributes_used;
		vertex_attributes = allocator.allocate<float>(vertex_count * attribute_count);
		rescale_attributes(vertex_attributes, vertex_attributes_data, vertex_count, vertex_attributes_stride, attribute_weights, attribute_count, attribute_remap, sparse_remap);
	}

	error_quadric* vertex_quadrics = allocator.allocate<error_quadric>(vertex_count);
	memset(vertex_quadrics, 0, vertex_count * sizeof(error_quadric));

	error_quadric* attribute_quadrics = NULL;
	quadric_gradient* attribute_gradients = NULL;
	quadric_gradient* volume_gradients = NULL;

	if (attribute_count) {
		attribute_quadrics = allocator.allocate<error_quadric>(vertex_count);
		memset(attribute_quadrics, 0, vertex_count * sizeof(error_quadric));

		attribute_gradients = allocator.allocate<quadric_gradient>(vertex_count * attribute_count);
		memset(attribute_gradients, 0, vertex_count * attribute_count * sizeof(quadric_gradient));

		if (options & mesh_simplify_internal_solve) {
			volume_gradients = allocator.allocate<quadric_gradient>(vertex_count);
			memset(volume_gradients, 0, vertex_count * sizeof(quadric_gradient));
		}
	}

	fill_face_quadrics(vertex_quadrics, volume_gradients, result, index_count, vertex_positions, remap);
	fill_vertex_quadrics(vertex_quadrics, vertex_positions, vertex_count, remap, options);
	fill_edge_quadrics(vertex_quadrics, result, index_count, vertex_positions, remap, vertex_kind, loop, loopback);

	if (attribute_count)
		fill_attribute_quadrics(attribute_quadrics, attribute_gradients, result, index_count, vertex_positions, vertex_attributes, attribute_count);

	unsigned int* components = NULL;
	float* component_errors = NULL;
	size_t component_count = 0;
	float component_nexterror = 0;

	if (options & mesh_simplify_prune_flag) {
		components = allocator.allocate<unsigned int>(vertex_count);
		component_count = build_components(components, vertex_count, result, index_count, remap);

		component_errors = allocator.allocate<float>(component_count * 4);
		measure_components(component_errors, component_count, components, vertex_positions, vertex_count);

		component_nexterror = FLT_MAX;
		for (size_t i = 0; i < component_count; ++i)
			component_nexterror = component_nexterror > component_errors[i] ? component_errors[i] : component_nexterror;

#if TRACE
		printf("components: %d (min error %e)\n", int(component_count), sqrtf(component_nexterror));
#endif
	}

#if TRACE
	size_t pass_count = 0;
#endif

	size_t collapse_capacity = bound_edge_collapses(adjacency, vertex_count, index_count, vertex_kind);

	edge_collapse* edge_collapses = allocator.allocate<edge_collapse>(collapse_capacity);
	unsigned int* collapse_order = allocator.allocate<unsigned int>(collapse_capacity);
	unsigned int* collapse_remap = allocator.allocate<unsigned int>(vertex_count);
	unsigned char* collapse_locked = allocator.allocate<unsigned char>(vertex_count);

	size_t result_count = index_count;
	float result_error = 0;
	float vertex_error = 0;

	float error_scale = (options & mesh_simplify_error_absolute_flag) ? vertex_scale : 1.f;
	float error_limit = (target_error * target_error) / (error_scale * error_scale);

	while (result_count > target_index_count) {

		update_edge_adjacency(adjacency, result, result_count, vertex_count, remap);

		size_t edge_collapse_count = pick_edge_collapses(edge_collapses, collapse_capacity, result, result_count, remap, vertex_kind, loop, loopback);
		assert(edge_collapse_count <= collapse_capacity);

		if (edge_collapse_count == 0)
			break;

#if TRACE
		printf("pass %d:%c", int(pass_count++), TRACE >= 2 ? '\n' : ' ');
#endif

		rank_edge_collapses(edge_collapses, edge_collapse_count, vertex_positions, vertex_attributes, vertex_quadrics, attribute_quadrics, attribute_gradients, attribute_count, remap, wedge, vertex_kind, loop, loopback);

		sort_edge_collapses(collapse_order, edge_collapses, edge_collapse_count);

		size_t triangle_collapse_goal = (result_count - target_index_count) / 3;

		for (size_t i = 0; i < vertex_count; ++i)
			collapse_remap[i] = unsigned(i);

		memset(collapse_locked, 0, vertex_count);

		size_t collapses = perform_edge_collapses(collapse_remap, collapse_locked, edge_collapses, edge_collapse_count, collapse_order, remap, wedge, vertex_kind, loop, loopback, vertex_positions, adjacency, triangle_collapse_goal, error_limit, result_error);

		if (collapses == 0)
			break;

		update_quadrics(collapse_remap, vertex_count, vertex_quadrics, volume_gradients, attribute_quadrics, attribute_gradients, attribute_count, vertex_positions, remap, vertex_error);

		vertex_error = attribute_count == 0 ? result_error : vertex_error;

		remap_edge_loops(loop, vertex_count, collapse_remap);
		remap_edge_loops(loopback, vertex_count, collapse_remap);

		result_count = remap_index_buffer(result, result_count, collapse_remap, remap);

		if ((options & mesh_simplify_prune_flag) && result_count > target_index_count && component_nexterror <= vertex_error)
			result_count = prune_components(result, result_count, components, component_errors, component_count, vertex_error, component_nexterror);
	}

	bool component_nextstale = true;

	while ((options & mesh_simplify_prune_flag) && result_count > target_index_count && component_nexterror <= error_limit) {
#if TRACE
		printf("pass %d: cleanup; ", int(pass_count++));
#endif

		float component_cutoff = component_nexterror * 1.5f < error_limit ? component_nexterror * 1.5f : error_limit;

		float component_maxerror = 0;
		for (size_t i = 0; i < component_count; ++i)
			if (component_errors[i] > component_maxerror && component_errors[i] <= component_cutoff)
				component_maxerror = component_errors[i];

		size_t new_count = prune_components(result, result_count, components, component_errors, component_count, component_cutoff, component_nexterror);
		if (new_count == result_count && !component_nextstale)
			break;

		component_nextstale = false;
		result_count = new_count;
		result_error = result_error < component_maxerror ? component_maxerror : result_error;
		vertex_error = vertex_error < component_maxerror ? component_maxerror : vertex_error;
	}

#if TRACE
	printf("result: %d triangles, error: %e (pos %.3e); total %d passes\n", int(result_count / 3), sqrtf(result_error), sqrtf(vertex_error), int(pass_count));
#endif

	if (options & mesh_simplify_internal_solve) {
		unsigned char* vertex_update = collapse_locked;
		memset(vertex_update, 0, vertex_count);

		for (size_t i = 0; i < result_count; ++i) {
			unsigned int v = result[i];

			vertex_update[remap[v]] = vertex_update[v] = 1;
		}

		update_edge_adjacency(adjacency, result, result_count, vertex_count, remap);

		solve_positions(vertex_positions, vertex_count, vertex_quadrics, volume_gradients, attribute_quadrics, attribute_gradients, attribute_count, remap, wedge, adjacency, vertex_kind, vertex_update);

		if (attribute_count)
			solve_attributes(vertex_positions, vertex_attributes, vertex_count, attribute_quadrics, attribute_gradients, attribute_count, remap, wedge, vertex_kind, vertex_update);

		finalize_vertices(const_cast<float*>(vertex_positions_data), vertex_positions_stride, const_cast<float*>(vertex_attributes_data), vertex_attributes_stride, attribute_weights, attribute_count, vertex_count, vertex_positions, vertex_attributes, sparse_remap, attribute_remap, vertex_scale, vertex_offset, vertex_kind, vertex_update, vertex_lock);
	}

	if ((options & mesh_simplify_internal_debug) && !sparse_remap) {
		assert(MARS_VERTEX_TYPE_COUNT <= 8 && vertex_count < (1 << 28));

		for (size_t i = 0; i < result_count; i += 3) {
			unsigned int a = result[i + 0], b = result[i + 1], c = result[i + 2];

			result[i + 0] |= (vertex_kind[a] << 28) | (unsigned(loop[a] == b || loopback[b] == a) << 31);
			result[i + 1] |= (vertex_kind[b] << 28) | (unsigned(loop[b] == c || loopback[c] == b) << 31);
			result[i + 2] |= (vertex_kind[c] << 28) | (unsigned(loop[c] == a || loopback[a] == c) << 31);
		}
	}

	if (sparse_remap)
		for (size_t i = 0; i < result_count; ++i)
			result[i] = sparse_remap[result[i]];

	if (out_result_error)
		*out_result_error = sqrtf(result_error) * error_scale;

	return result_count;
}

size_t mesh_simplify(unsigned int* destination, const unsigned int* indices, size_t index_count, const float* vertex_positions_data, size_t vertex_count, size_t vertex_positions_stride, size_t target_index_count, float target_error, unsigned int options, float* out_result_error) {
	assert((options & mesh_simplify_internal_solve) == 0);

	return mesh_simplify_edge(destination, indices, index_count, vertex_positions_data, vertex_count, vertex_positions_stride, NULL, 0, NULL, 0, NULL, target_index_count, target_error, options, out_result_error);
}

size_t mesh_simplify_with_attributes(unsigned int* destination, const unsigned int* indices, size_t index_count, const float* vertex_positions_data, size_t vertex_count, size_t vertex_positions_stride, const float* vertex_attributes_data, size_t vertex_attributes_stride, const float* attribute_weights, size_t attribute_count, const unsigned char* vertex_lock, size_t target_index_count, float target_error, unsigned int options, float* out_result_error) {
	assert((options & mesh_simplify_internal_solve) == 0);

	return mesh_simplify_edge(destination, indices, index_count, vertex_positions_data, vertex_count, vertex_positions_stride, vertex_attributes_data, vertex_attributes_stride, attribute_weights, attribute_count, vertex_lock, target_index_count, target_error, options, out_result_error);
}

size_t mesh_simplify_with_update(unsigned int* indices, size_t index_count, float* vertex_positions_data, size_t vertex_count, size_t vertex_positions_stride, float* vertex_attributes_data, size_t vertex_attributes_stride, const float* attribute_weights, size_t attribute_count, const unsigned char* vertex_lock, size_t target_index_count, float target_error, unsigned int options, float* out_result_error) {
	return mesh_simplify_edge(indices, indices, index_count, vertex_positions_data, vertex_count, vertex_positions_stride, vertex_attributes_data, vertex_attributes_stride, attribute_weights, attribute_count, vertex_lock, target_index_count, target_error, options | mesh_simplify_internal_solve, out_result_error);
}

size_t mesh_simplify_sloppy(unsigned int* destination, const unsigned int* indices, size_t index_count, const float* vertex_positions_data, size_t vertex_count, size_t vertex_positions_stride, const unsigned char* vertex_lock, size_t target_index_count, float target_error, float* out_result_error) {
	using namespace mars::mesh::core;

	assert(index_count % 3 == 0);
	assert(vertex_positions_stride >= 12 && vertex_positions_stride <= 256);
	assert(vertex_positions_stride % sizeof(float) == 0);
	assert(target_index_count <= index_count);

	size_t target_cell_count = target_index_count / 6;

	array_stack_heap_allocator<24> allocator;

	mars::vector3<float>* vertex_positions = allocator.allocate<mars::vector3<float>>(vertex_count);
	rescale_positions(vertex_positions, vertex_positions_data, vertex_count, vertex_positions_stride);

#if TRACE
	printf("source: %d vertices, %d triangles\n", int(vertex_count), int(index_count / 3));
	printf("target: %d cells, %d triangles\n", int(target_cell_count), int(target_index_count / 3));
#endif

	unsigned int* vertex_ids = allocator.allocate<unsigned int>(vertex_count);

	const int k_interpolation_passes = 5;

	int min_grid = int(1.f / (target_error < 1e-3f ? 1e-3f : (target_error < 1.f ? target_error : 1.f)));
	int max_grid = 1025;
	size_t min_triangles = 0;
	size_t max_triangles = index_count / 3;

	if (min_grid > 1 || vertex_lock) {
		compute_vertex_ids(vertex_ids, vertex_positions, vertex_lock, vertex_count, min_grid);
		min_triangles = count_triangles(vertex_ids, indices, index_count);
	}

	int next_grid_size = int(sqrtf(float(target_cell_count)) + 0.5f);

	for (int pass = 0; pass < 10 + k_interpolation_passes; ++pass) {
		if (min_triangles >= target_index_count / 3 || max_grid - min_grid <= 1)
			break;

		int grid_size = next_grid_size;
		grid_size = (grid_size <= min_grid) ? min_grid + 1 : (grid_size >= max_grid ? max_grid - 1 : grid_size);

		compute_vertex_ids(vertex_ids, vertex_positions, vertex_lock, vertex_count, grid_size);
		size_t triangles = count_triangles(vertex_ids, indices, index_count);

#if TRACE
		printf("pass %d (%s): grid size %d, triangles %d, %s\n",
		       pass, (pass == 0) ? "guess" : (pass <= k_interpolation_passes ? "lerp" : "binary"),
		       grid_size, int(triangles),
		       (triangles <= target_index_count / 3) ? "under" : "over");
#endif

		float tip = interpolate(float(size_t(target_index_count / 3)), float(min_grid), float(min_triangles), float(grid_size), float(triangles), float(max_grid), float(max_triangles));

		if (triangles <= target_index_count / 3) {
			min_grid = grid_size;
			min_triangles = triangles;
		} else {
			max_grid = grid_size;
			max_triangles = triangles;
		}

		next_grid_size = (pass < k_interpolation_passes) ? int(tip + 0.5f) : (min_grid + max_grid) / 2;
	}

	if (min_triangles == 0) {
		if (out_result_error)
			*out_result_error = 1.f;

		return 0;
	}

	size_t table_size = hash_buckets2(vertex_count);
	unsigned int* table = allocator.allocate<unsigned int>(table_size);

	unsigned int* vertex_cells = allocator.allocate<unsigned int>(vertex_count);

	compute_vertex_ids(vertex_ids, vertex_positions, vertex_lock, vertex_count, min_grid);
	size_t cell_count = fill_vertex_cells(table, table_size, vertex_cells, vertex_ids, vertex_count);

	error_quadric* cell_quadrics = allocator.allocate<error_quadric>(cell_count);
	memset(cell_quadrics, 0, cell_count * sizeof(error_quadric));

	fill_cell_quadrics(cell_quadrics, indices, index_count, vertex_positions, vertex_cells);

	unsigned int* cell_remap = allocator.allocate<unsigned int>(cell_count);
	float* cell_errors = allocator.allocate<float>(cell_count);

	fill_cell_remap(cell_remap, cell_errors, cell_count, vertex_cells, cell_quadrics, vertex_positions, vertex_count);

	float result_error = 0.f;

	for (size_t i = 0; i < cell_count; ++i)
		result_error = result_error < cell_errors[i] ? cell_errors[i] : result_error;

	size_t tritable_size = hash_buckets2(min_triangles);
	unsigned int* tritable = allocator.allocate<unsigned int>(tritable_size);

	size_t write = filter_triangles(destination, tritable, tritable_size, indices, index_count, vertex_cells, cell_remap);

#if TRACE
	printf("result: grid size %d, %d cells, %d triangles (%d unfiltered), error %e\n", min_grid, int(cell_count), int(write / 3), int(min_triangles), sqrtf(result_error));
#endif

	if (out_result_error)
		*out_result_error = sqrtf(result_error);

	return write;
}

size_t mesh_simplify_prune(unsigned int* destination, const unsigned int* indices, size_t index_count, const float* vertex_positions_data, size_t vertex_count, size_t vertex_positions_stride, float target_error) {
	using namespace mars::mesh::core;

	assert(index_count % 3 == 0);
	assert(vertex_positions_stride >= 12 && vertex_positions_stride <= 256);
	assert(vertex_positions_stride % sizeof(float) == 0);
	assert(target_error >= 0);

	array_stack_heap_allocator<24> allocator;

	unsigned int* result = destination;
	if (result != indices)
		memcpy(result, indices, index_count * sizeof(unsigned int));

	unsigned int* remap = allocator.allocate<unsigned int>(vertex_count);
	build_position_remap(remap, NULL, vertex_positions_data, vertex_count, vertex_positions_stride, NULL, allocator);

	mars::vector3<float>* vertex_positions = allocator.allocate<mars::vector3<float>>(vertex_count);
	rescale_positions(vertex_positions, vertex_positions_data, vertex_count, vertex_positions_stride, NULL);

	unsigned int* components = allocator.allocate<unsigned int>(vertex_count);
	size_t component_count = build_components(components, vertex_count, indices, index_count, remap);

	float* component_errors = allocator.allocate<float>(component_count * 4);
	measure_components(component_errors, component_count, components, vertex_positions, vertex_count);

	float component_nexterror = 0;
	size_t result_count = prune_components(result, index_count, components, component_errors, component_count, target_error * target_error, component_nexterror);

	return result_count;
}

size_t mesh_simplify_points(unsigned int* destination, const float* vertex_positions_data, size_t vertex_count, size_t vertex_positions_stride, const float* vertex_colors, size_t vertex_colors_stride, float color_weight, size_t target_vertex_count) {
	using namespace mars::mesh::core;

	assert(vertex_positions_stride >= 12 && vertex_positions_stride <= 256);
	assert(vertex_positions_stride % sizeof(float) == 0);
	assert(vertex_colors_stride == 0 || (vertex_colors_stride >= 12 && vertex_colors_stride <= 256));
	assert(vertex_colors_stride % sizeof(float) == 0);
	assert(vertex_colors == NULL || vertex_colors_stride != 0);
	assert(target_vertex_count <= vertex_count);

	size_t target_cell_count = target_vertex_count;

	if (target_cell_count == 0)
		return 0;

	array_stack_heap_allocator<24> allocator;

	mars::vector3<float>* vertex_positions = allocator.allocate<mars::vector3<float>>(vertex_count);
	rescale_positions(vertex_positions, vertex_positions_data, vertex_count, vertex_positions_stride);

#if TRACE
	printf("source: %d vertices\n", int(vertex_count));
	printf("target: %d cells\n", int(target_cell_count));
#endif

	unsigned int* vertex_ids = allocator.allocate<unsigned int>(vertex_count);

	size_t table_size = hash_buckets2(vertex_count);
	unsigned int* table = allocator.allocate<unsigned int>(table_size);

	const int k_interpolation_passes = 5;

	int min_grid = 0;
	int max_grid = 1025;
	size_t min_vertices = 0;
	size_t max_vertices = vertex_count;

	int next_grid_size = int(sqrtf(float(target_cell_count)) + 0.5f);

	for (int pass = 0; pass < 10 + k_interpolation_passes; ++pass) {
		assert(min_vertices < target_vertex_count);
		assert(max_grid - min_grid > 1);

		int grid_size = next_grid_size;
		grid_size = (grid_size <= min_grid) ? min_grid + 1 : (grid_size >= max_grid ? max_grid - 1 : grid_size);

		compute_vertex_ids(vertex_ids, vertex_positions, NULL, vertex_count, grid_size);
		size_t vertices = count_vertex_cells(table, table_size, vertex_ids, vertex_count);

#if TRACE
		printf("pass %d (%s): grid size %d, vertices %d, %s\n",
		       pass, (pass == 0) ? "guess" : (pass <= k_interpolation_passes ? "lerp" : "binary"),
		       grid_size, int(vertices),
		       (vertices <= target_vertex_count) ? "under" : "over");
#endif

		float tip = interpolate(float(target_vertex_count), float(min_grid), float(min_vertices), float(grid_size), float(vertices), float(max_grid), float(max_vertices));

		if (vertices <= target_vertex_count) {
			min_grid = grid_size;
			min_vertices = vertices;
		} else {
			max_grid = grid_size;
			max_vertices = vertices;
		}

		if (vertices == target_vertex_count || max_grid - min_grid <= 1)
			break;

		next_grid_size = (pass < k_interpolation_passes) ? int(tip + 0.5f) : (min_grid + max_grid) / 2;
	}

	if (min_vertices == 0)
		return 0;

	unsigned int* vertex_cells = allocator.allocate<unsigned int>(vertex_count);

	compute_vertex_ids(vertex_ids, vertex_positions, NULL, vertex_count, min_grid);
	size_t cell_count = fill_vertex_cells(table, table_size, vertex_cells, vertex_ids, vertex_count);

	point_reservoir* cell_reservoirs = allocator.allocate<point_reservoir>(cell_count);
	memset(cell_reservoirs, 0, cell_count * sizeof(point_reservoir));

	fill_cell_reservoirs(cell_reservoirs, cell_count, vertex_positions, vertex_colors, vertex_colors_stride, vertex_count, vertex_cells);

	unsigned int* cell_remap = allocator.allocate<unsigned int>(cell_count);
	float* cell_errors = allocator.allocate<float>(cell_count);

	float color_weight_scaled = color_weight * (min_grid == 1 ? 1.f : 1.f / (min_grid - 1));

	fill_cell_remap(cell_remap, cell_errors, cell_count, vertex_cells, cell_reservoirs, vertex_positions, vertex_colors, vertex_colors_stride, color_weight_scaled * color_weight_scaled, vertex_count);

	assert(cell_count <= target_vertex_count);
	memcpy(destination, cell_remap, sizeof(unsigned int) * cell_count);

#if TRACE

	float result_error = 0.f;

	for (size_t i = 0; i < cell_count; ++i)
		result_error = result_error < cell_errors[i] ? cell_errors[i] : result_error;

	printf("result: %d cells, %e error\n", int(cell_count), sqrtf(result_error));
#endif

	return cell_count;
}

float mesh_simplify_scale(const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride) {
	using namespace mars::mesh::core;

	assert(vertex_positions_stride >= 12 && vertex_positions_stride <= 256);
	assert(vertex_positions_stride % sizeof(float) == 0);

	float extent = rescale_positions(NULL, vertex_positions, vertex_count, vertex_positions_stride);

	return extent;
}

} // namespace mars::mesh

