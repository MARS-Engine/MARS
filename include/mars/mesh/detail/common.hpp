#pragma once

#include <mars/math/vector2.hpp>
#include <mars/math/vector3.hpp>
#include <mars/math/vector4.hpp>
#include <mars/mesh/detail/raw_api.hpp>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace mars::mesh::detail {

struct working_vertex {
	std::vector<std::byte> bytes;
	mars::vector3<float> position = {};
	mars::vector3<float> normal = {0.0f, 1.0f, 0.0f};
	mars::vector2<float> uv = {};
	mars::vector4<float> tangent = {1.0f, 0.0f, 0.0f, 1.0f};
	uint8_t flags = 0;
	bool active = true;
};

constexpr uint8_t vertex_flag_locked = 1u << 0;
constexpr uint8_t vertex_flag_protect = 1u << 1;

template <typename type>
type load_attribute(const std::byte* data, size_t offset) {
	type value{};
	std::memcpy(&value, data + offset, sizeof(type));
	return value;
}

template <typename type>
void store_attribute(std::byte* data, size_t offset, const type& value) {
	std::memcpy(data + offset, &value, sizeof(type));
}

inline mars::vector3<uint32_t> make_position_key(const mars::vector3<float>& position) {
	return {
	    std::bit_cast<uint32_t>(position.x),
	    std::bit_cast<uint32_t>(position.y),
	    std::bit_cast<uint32_t>(position.z),
	};
}

std::vector<working_vertex> make_working_vertices(const raw_simplify_input& input);
std::vector<uint8_t> build_vertex_flags(const raw_simplify_input& input, const std::vector<working_vertex>& vertices);
void write_vertex_bytes(working_vertex& vertex, const raw_simplify_input& input);
void renormalize_vertex(working_vertex& vertex, const raw_simplify_input& input);

} // namespace mars::mesh::detail
