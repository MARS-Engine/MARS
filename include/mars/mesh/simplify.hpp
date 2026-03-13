#pragma once

#include <mars/mesh/detail/raw_api.hpp>
#include <mars/mesh/mesh_reflection.hpp>

#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

namespace mars::mesh {

struct simplify_settings {
	std::size_t target_triangle_count = 0;
	std::span<const uint8_t> locked_mask = {};
	float normal_weight = 0.5f;
	float uv_weight = 16.0f;
	float regularization_weight = 1e-4f;
	bool allow_permissive_retry = true;
};

template <typename vertex_type>
struct simplify_result {
	std::vector<vertex_type> vertices;
	std::vector<uint32_t> indices;
	float max_collapse_cost_sqrt = 0.0f;
	bool reached_target = false;
	bool used_permissive_retry = false;
};

template <typename vertex_type>
simplify_result<vertex_type> simplify_mesh(
	std::span<const vertex_type> vertices,
	std::span<const uint32_t> indices,
	const simplify_settings& settings
) {
	detail::vertex_layout<vertex_type>::validate();

	const detail::raw_simplify_input input = {
		.vertex_bytes = std::as_bytes(vertices),
		.vertex_stride = sizeof(vertex_type),
		.indices = indices,
		.target_triangle_count = settings.target_triangle_count,
		.position_offset = detail::vertex_layout<vertex_type>::position_offset(),
		.position_has_w = detail::vertex_layout<vertex_type>::has_vec4_position,
		.has_normal = detail::vertex_layout<vertex_type>::has_normal,
		.normal_offset = detail::vertex_layout<vertex_type>::normal_offset(),
		.has_uv = detail::vertex_layout<vertex_type>::has_uv,
		.uv_offset = detail::vertex_layout<vertex_type>::uv_offset(),
		.has_tangent = detail::vertex_layout<vertex_type>::has_tangent,
		.tangent_offset = detail::vertex_layout<vertex_type>::tangent_offset(),
		.locked_mask = settings.locked_mask,
		.normal_weight = settings.normal_weight,
		.uv_weight = settings.uv_weight,
		.regularization_weight = settings.regularization_weight,
		.allow_permissive_retry = settings.allow_permissive_retry,
	};

	const detail::raw_simplify_result raw = detail::simplify_mesh_raw(input);

	simplify_result<vertex_type> result;
	result.indices = raw.indices;
	result.max_collapse_cost_sqrt = raw.max_collapse_cost_sqrt;
	result.reached_target = raw.reached_target;
	result.used_permissive_retry = raw.used_permissive_retry;

	if (!raw.vertex_bytes.empty()) {
		const std::size_t vertex_count = raw.vertex_bytes.size() / sizeof(vertex_type);
		result.vertices.resize(vertex_count);
		std::memcpy(result.vertices.data(), raw.vertex_bytes.data(), vertex_count * sizeof(vertex_type));
	}

	return result;
}

} // namespace mars::mesh
