#pragma once

#include <mars/math/vector2.hpp>
#include <mars/math/vector3.hpp>
#include <mars/math/vector4.hpp>
#include <mars/meta.hpp>

#include <cstddef>
#include <meta>
#include <type_traits>

namespace mars::mesh {

struct position {};
struct normal {};
struct uv {};
struct texcoord {};
struct tangent {};
struct ignore {};

namespace detail {

enum class vertex_semantic {
	none,
	position,
	normal,
	uv,
	tangent,
	ignore,
};

template <auto member>
consteval vertex_semantic semantic_of_member() {
	if (mars::meta::has_annotation<position>(member))
		return vertex_semantic::position;
	if (mars::meta::has_annotation<normal>(member))
		return vertex_semantic::normal;
	if (mars::meta::has_annotation<uv>(member) || mars::meta::has_annotation<texcoord>(member))
		return vertex_semantic::uv;
	if (mars::meta::has_annotation<tangent>(member))
		return vertex_semantic::tangent;
	if (mars::meta::has_annotation<ignore>(member))
		return vertex_semantic::ignore;

	constexpr auto name = std::meta::identifier_of(member);
	if constexpr (name == "position")
		return vertex_semantic::position;
	else if constexpr (name == "normal")
		return vertex_semantic::normal;
	else if constexpr (name == "uv" || name == "texcoord")
		return vertex_semantic::uv;
	else if constexpr (name == "tangent")
		return vertex_semantic::tangent;
	else
		return vertex_semantic::none;
}

template <typename vertex_type, vertex_semantic wanted>
consteval std::meta::info find_semantic_member() {
	std::meta::info result{};
	constexpr auto ctx = std::meta::access_context::current();

	template for (constexpr auto member : std::define_static_array(std::meta::nonstatic_data_members_of(^^vertex_type, ctx))) {
		if (result == std::meta::info{} && semantic_of_member<member>() == wanted)
			result = member;
	}

	return result;
}

template <typename vertex_type, auto member>
consteval std::size_t member_offset() {
	static_assert(std::is_standard_layout_v<vertex_type>, "mars::mesh vertex_type must be standard-layout");
	static_assert(std::is_trivially_copyable_v<vertex_type>, "mars::mesh vertex_type must be trivially copyable");
	static_assert(std::is_default_constructible_v<vertex_type>, "mars::mesh vertex_type must be default constructible");

	return std::meta::offset_of(member).bytes;
}

template <typename field_type, typename expected_type>
consteval void validate_semantic_type(const char* semantic_name) {
	static_assert(std::is_same_v<field_type, expected_type>, "mars::mesh semantic field has the wrong type");
	(void)semantic_name;
}

template <typename vertex_type>
struct vertex_layout {
	static constexpr auto position_member = find_semantic_member<vertex_type, vertex_semantic::position>();
	static constexpr auto normal_member = find_semantic_member<vertex_type, vertex_semantic::normal>();
	static constexpr auto uv_member = find_semantic_member<vertex_type, vertex_semantic::uv>();
	static constexpr auto tangent_member = find_semantic_member<vertex_type, vertex_semantic::tangent>();

	static_assert(position_member != std::meta::info{}, "mars::mesh vertex_type must expose a position field");

	using position_type = typename[:std::meta::type_of(position_member):];
	static_assert(std::is_same_v<position_type, mars::vector3<float>>, "mars::mesh position field must be mars::vector3<float>");

	static constexpr bool has_normal = normal_member != std::meta::info{};
	static constexpr bool has_uv = uv_member != std::meta::info{};
	static constexpr bool has_tangent = tangent_member != std::meta::info{};

	static constexpr std::size_t stride = sizeof(vertex_type);

	static consteval std::size_t position_offset() {
		return member_offset<vertex_type, position_member>();
	}

	static consteval std::size_t normal_offset() {
		if constexpr (has_normal)
			return member_offset<vertex_type, normal_member>();
		return std::size_t(0);
	}

	static consteval std::size_t uv_offset() {
		if constexpr (has_uv)
			return member_offset<vertex_type, uv_member>();
		return std::size_t(0);
	}

	static consteval std::size_t tangent_offset() {
		if constexpr (has_tangent)
			return member_offset<vertex_type, tangent_member>();
		return std::size_t(0);
	}

	static consteval void validate() {
		if constexpr (has_normal) {
			using normal_type = typename[:std::meta::type_of(normal_member):];
			static_assert(std::is_same_v<normal_type, mars::vector3<float>>, "mars::mesh normal field must be mars::vector3<float>");
		}
		if constexpr (has_uv) {
			using uv_type = typename[:std::meta::type_of(uv_member):];
			static_assert(std::is_same_v<uv_type, mars::vector2<float>>, "mars::mesh uv/texcoord field must be mars::vector2<float>");
		}
		if constexpr (has_tangent) {
			using tangent_type = typename[:std::meta::type_of(tangent_member):];
			static_assert(std::is_same_v<tangent_type, mars::vector4<float>>, "mars::mesh tangent field must be mars::vector4<float>");
		}
	}
};

} // namespace detail

} // namespace mars::mesh
