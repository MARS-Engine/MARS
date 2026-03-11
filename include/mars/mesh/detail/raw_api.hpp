#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace mars::mesh {

struct meshlet;
struct meshlet_build_result;

namespace detail {

struct raw_meshlet_build_input {
	std::span<const std::byte> vertex_bytes = {};
	std::size_t vertex_stride = 0;
	std::size_t position_offset = 0;
	std::span<const uint32_t> indices = {};
	uint32_t max_vertices = 256;
	uint32_t max_triangles = 128;
};

struct raw_simplify_input {
	std::span<const std::byte> vertex_bytes = {};
	std::size_t vertex_stride = 0;
	std::span<const uint32_t> indices = {};
	std::size_t target_triangle_count = 0;
	std::size_t position_offset = 0;
	bool position_has_w = false;
	bool has_normal = false;
	std::size_t normal_offset = 0;
	bool has_uv = false;
	std::size_t uv_offset = 0;
	bool has_tangent = false;
	std::size_t tangent_offset = 0;
	std::span<const uint8_t> locked_mask = {};
	float normal_weight = 0.5f;
	float uv_weight = 16.0f;
	float regularization_weight = 1e-4f;
	bool allow_permissive_retry = true;
};

struct raw_simplify_result {
	std::vector<std::byte> vertex_bytes;
	std::size_t vertex_stride = 0;
	std::vector<uint32_t> indices;
	float max_collapse_cost_sqrt = 0.0f;
	bool reached_target = false;
	bool used_permissive_retry = false;
};

meshlet_build_result build_meshlets_raw(const raw_meshlet_build_input& input);
raw_simplify_result simplify_mesh_raw(const raw_simplify_input& input);

} // namespace detail

} // namespace mars::mesh
