#include <mars/mesh/detail/common.hpp>

#include <cmath>
#include <unordered_map>

namespace mars::mesh::detail {
namespace {

bool nearly_equal(float left, float right, float epsilon = 1e-6f) {
	return std::abs(left - right) <= epsilon;
}

bool uv_seam(const working_vertex& left, const working_vertex& right) {
	return !nearly_equal(left.uv.x, right.uv.x) || !nearly_equal(left.uv.y, right.uv.y);
}

bool normal_seam(const working_vertex& left, const working_vertex& right) {
	const auto left_normal = mars::normalize_or(left.normal, mars::vector3<float>{0.0f, 1.0f, 0.0f});
	const auto right_normal = mars::normalize_or(right.normal, mars::vector3<float>{0.0f, 1.0f, 0.0f});
	return mars::dot(left_normal, right_normal) < 0.9999f;
}

bool tangent_handedness_seam(const working_vertex& left, const working_vertex& right) {
	return (left.tangent.w < 0.0f) != (right.tangent.w < 0.0f);
}

} // namespace

std::vector<working_vertex> make_working_vertices(const raw_simplify_input& input) {
	const size_t vertex_count = input.vertex_stride == 0 ? 0 : input.vertex_bytes.size() / input.vertex_stride;
	std::vector<working_vertex> vertices(vertex_count);
	for (size_t i = 0; i < vertex_count; ++i) {
		const std::byte* source = input.vertex_bytes.data() + i * input.vertex_stride;
		vertices[i].bytes.assign(source, source + input.vertex_stride);
		if (input.position_has_w) {
			const mars::vector4<float> position = load_attribute<mars::vector4<float>>(source, input.position_offset);
			vertices[i].position = {position.x, position.y, position.z};
		} else {
			vertices[i].position = load_attribute<mars::vector3<float>>(source, input.position_offset);
		}
		if (input.has_normal)
			vertices[i].normal = load_attribute<mars::vector3<float>>(source, input.normal_offset);
		if (input.has_uv)
			vertices[i].uv = load_attribute<mars::vector2<float>>(source, input.uv_offset);
		if (input.has_tangent)
			vertices[i].tangent = load_attribute<mars::vector4<float>>(source, input.tangent_offset);
	}
	return vertices;
}

std::vector<uint8_t> build_vertex_flags(
	const raw_simplify_input& input,
	const std::vector<working_vertex>& vertices
) {
	std::vector<uint8_t> flags(vertices.size(), 0u);
	std::unordered_map<mars::vector3<uint32_t>, uint32_t> representative_vertices;
	representative_vertices.reserve(vertices.size());

	for (uint32_t i = 0; i < vertices.size(); ++i) {
		if (!input.locked_mask.empty() && i < input.locked_mask.size() && input.locked_mask[i] != 0)
			flags[i] |= vertex_flag_locked;

		auto [it, inserted] = representative_vertices.emplace(make_position_key(vertices[i].position), i);
		if (!inserted) {
			if ((input.has_uv && uv_seam(vertices[it->second], vertices[i])) ||
				(input.has_normal && normal_seam(vertices[it->second], vertices[i])) ||
				(input.has_tangent && tangent_handedness_seam(vertices[it->second], vertices[i]))) {
				flags[i] |= vertex_flag_protect;
				flags[it->second] |= vertex_flag_protect;
			}
		}
	}

	for (uint32_t i = 0; i < vertices.size(); ++i) {
		const auto it = representative_vertices.find(make_position_key(vertices[i].position));
		if (it != representative_vertices.end())
			flags[i] |= (flags[it->second] & vertex_flag_protect);
	}

	return flags;
}

void write_vertex_bytes(working_vertex& vertex, const raw_simplify_input& input) {
	if (input.position_has_w) {
		mars::vector4<float> position = load_attribute<mars::vector4<float>>(vertex.bytes.data(), input.position_offset);
		position.x = vertex.position.x;
		position.y = vertex.position.y;
		position.z = vertex.position.z;
		store_attribute(vertex.bytes.data(), input.position_offset, position);
	} else {
		store_attribute(vertex.bytes.data(), input.position_offset, vertex.position);
	}
	if (input.has_normal)
		store_attribute(vertex.bytes.data(), input.normal_offset, vertex.normal);
	if (input.has_uv)
		store_attribute(vertex.bytes.data(), input.uv_offset, vertex.uv);
	if (input.has_tangent)
		store_attribute(vertex.bytes.data(), input.tangent_offset, vertex.tangent);
}

void renormalize_vertex(working_vertex& vertex, const raw_simplify_input& input) {
	if (input.has_normal)
		vertex.normal = mars::normalize_or(vertex.normal, mars::vector3<float>{0.0f, 1.0f, 0.0f});

	if (input.has_tangent) {
		mars::vector3<float> tangent = {vertex.tangent.x, vertex.tangent.y, vertex.tangent.z};
		if (input.has_normal)
			tangent = tangent - vertex.normal * mars::dot(vertex.normal, tangent);

		tangent = mars::normalize_or(tangent, mars::vector3<float>{1.0f, 0.0f, 0.0f});
		vertex.tangent = {tangent.x, tangent.y, tangent.z, (vertex.tangent.w < 0.0f) ? -1.0f : 1.0f};
	}

	write_vertex_bytes(vertex, input);
}

} // namespace mars::mesh::detail
