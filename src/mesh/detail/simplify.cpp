#include <mars/mesh/detail/common.hpp>
#include <mars/mesh/mesh.hpp>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <vector>

namespace mars::mesh {

detail::raw_simplify_result detail::simplify_mesh_raw(const raw_simplify_input& input) {
	detail::raw_simplify_result passthrough;
	passthrough.vertex_stride = input.vertex_stride;

	if (input.vertex_stride == 0 || input.vertex_bytes.empty() || input.indices.empty() || input.indices.size() % 3 != 0) {
		if (!input.vertex_bytes.empty())
			passthrough.vertex_bytes.assign(
				input.vertex_bytes.begin(),
				input.vertex_bytes.end()
			);
		passthrough.indices.assign(input.indices.begin(), input.indices.end());
		return passthrough;
	}

	const size_t vertex_count = input.vertex_bytes.size() / input.vertex_stride;
	if (vertex_count == 0) {
		passthrough.indices.assign(input.indices.begin(), input.indices.end());
		return passthrough;
	}

	const size_t source_triangle_count = input.indices.size() / 3;
	if (source_triangle_count <= input.target_triangle_count) {
		passthrough.vertex_bytes.assign(
			input.vertex_bytes.begin(),
			input.vertex_bytes.end()
		);
		passthrough.indices.assign(input.indices.begin(), input.indices.end());
		passthrough.reached_target = true;
		return passthrough;
	}

	std::vector<detail::working_vertex> working_vertices = detail::make_working_vertices(input);
	std::vector<uint8_t> vertex_locks = detail::build_vertex_flags(input, working_vertices);

	std::vector<uint32_t> indices(input.indices.begin(), input.indices.end());
	std::vector<float> positions(vertex_count * 3);

	size_t attribute_count = 0;
	if (input.has_normal)
		attribute_count += 3;
	if (input.has_uv)
		attribute_count += 2;
	if (input.has_tangent)
		attribute_count += 4;

	std::vector<float> attributes(vertex_count * attribute_count, 0.0f);
	std::vector<float> attribute_weights(attribute_count, 0.0f);

	for (size_t i = 0; i < vertex_count; ++i) {
		positions[i * 3 + 0] = working_vertices[i].position.x;
		positions[i * 3 + 1] = working_vertices[i].position.y;
		positions[i * 3 + 2] = working_vertices[i].position.z;

		size_t attribute_offset = i * attribute_count;
		size_t weight_offset = 0;

		if (input.has_normal) {
			attributes[attribute_offset + 0] = working_vertices[i].normal.x;
			attributes[attribute_offset + 1] = working_vertices[i].normal.y;
			attributes[attribute_offset + 2] = working_vertices[i].normal.z;
			attribute_weights[weight_offset + 0] = input.normal_weight;
			attribute_weights[weight_offset + 1] = input.normal_weight;
			attribute_weights[weight_offset + 2] = input.normal_weight;
			attribute_offset += 3;
			weight_offset += 3;
		}

		if (input.has_uv) {
			attributes[attribute_offset + 0] = working_vertices[i].uv.x;
			attributes[attribute_offset + 1] = working_vertices[i].uv.y;
			attribute_weights[weight_offset + 0] = input.uv_weight;
			attribute_weights[weight_offset + 1] = input.uv_weight;
			attribute_offset += 2;
			weight_offset += 2;
		}

		if (input.has_tangent) {
			attributes[attribute_offset + 0] = working_vertices[i].tangent.x;
			attributes[attribute_offset + 1] = working_vertices[i].tangent.y;
			attributes[attribute_offset + 2] = working_vertices[i].tangent.z;
			attributes[attribute_offset + 3] = working_vertices[i].tangent.w;
		}
	}

	auto run_meshoptimizer = [&](unsigned int meshoptimizer_options, bool used_retry) -> detail::raw_simplify_result {
		std::vector<uint32_t> pass_indices = indices;
		std::vector<float> pass_positions = positions;
		std::vector<float> pass_attributes = attributes;
		float result_error = 0.0f;

		const size_t simplified_index_count = mesh_simplify_with_update(
			pass_indices.data(),
			pass_indices.size(),
			pass_positions.data(),
			vertex_count,
			sizeof(float) * 3,
			pass_attributes.empty() ? nullptr : pass_attributes.data(),
			attribute_count ? sizeof(float) * attribute_count : 0,
			attribute_count ? attribute_weights.data() : nullptr,
			attribute_count,
			vertex_locks.empty() ? nullptr : vertex_locks.data(),
			input.target_triangle_count * 3,
			FLT_MAX,
			meshoptimizer_options,
			&result_error
		);

		detail::raw_simplify_result output;
		output.vertex_stride = input.vertex_stride;
		output.max_collapse_cost_sqrt = std::sqrt(std::max(result_error, 0.0f));
		output.reached_target = simplified_index_count != 0 && simplified_index_count <= input.target_triangle_count * 3;
		output.used_permissive_retry = used_retry;

		if (simplified_index_count == 0 || simplified_index_count > pass_indices.size())
			return output;

		pass_indices.resize(simplified_index_count);
		std::vector<uint32_t> remap(vertex_count, ~0u);
		uint32_t next_index = 0;
		for (uint32_t index : pass_indices)
			if (index < remap.size() && remap[index] == ~0u)
				remap[index] = next_index++;

		output.vertex_bytes.resize(static_cast<size_t>(next_index) * input.vertex_stride);
		for (size_t i = 0; i < vertex_count; ++i) {
			if (remap[i] == ~0u)
				continue;

			detail::working_vertex vertex = working_vertices[i];
			vertex.position = {
				pass_positions[i * 3 + 0],
				pass_positions[i * 3 + 1],
				pass_positions[i * 3 + 2],
			};

			size_t attribute_offset = i * attribute_count;
			if (input.has_normal) {
				vertex.normal = {
					pass_attributes[attribute_offset + 0],
					pass_attributes[attribute_offset + 1],
					pass_attributes[attribute_offset + 2],
				};
				attribute_offset += 3;
			}
			if (input.has_uv) {
				vertex.uv = {
					pass_attributes[attribute_offset + 0],
					pass_attributes[attribute_offset + 1],
				};
				attribute_offset += 2;
			}
			if (input.has_tangent)
				vertex.tangent = {
					pass_attributes[attribute_offset + 0],
					pass_attributes[attribute_offset + 1],
					pass_attributes[attribute_offset + 2],
					pass_attributes[attribute_offset + 3],
				};

			detail::renormalize_vertex(vertex, input);
			std::memcpy(
				output.vertex_bytes.data() + static_cast<size_t>(remap[i]) * input.vertex_stride,
				vertex.bytes.data(),
				input.vertex_stride
			);
		}

		output.indices.reserve(pass_indices.size());
		for (uint32_t index : pass_indices)
			output.indices.push_back(remap[index]);
		return output;
	};

	const unsigned int base_meshoptimizer_options =
		mesh_simplify_sparse_flag |
		mesh_simplify_error_absolute_flag |
		mesh_simplify_regularize_flag;

	detail::raw_simplify_result result = run_meshoptimizer(base_meshoptimizer_options, false);
	if (!result.reached_target && input.allow_permissive_retry)
		result = run_meshoptimizer(base_meshoptimizer_options | mesh_simplify_permissive_flag, true);

	if (result.vertex_bytes.empty() || result.indices.empty())
		return passthrough;

	return result;
}

} // namespace mars::mesh
