#pragma once

#include <mars/imgui/annotation.hpp>
#include <mars/mesh/detail/raw_api.hpp>
#include <mars/mesh/mesh_reflection.hpp>

#include <cstdint>
#include <span>
#include <vector>

namespace mars::mesh {

struct meshlet {
	std::vector<uint32_t> unique_vertex_indices;
	std::vector<uint8_t> local_indices;
};

struct meshlet_build_settings {
	[[= mars::meta::display("Max Vertices")]]
	    [[= mars::imgui::slider(3u, 256u)]] uint32_t max_vertices = 256;

	[[= mars::meta::display("Max Triangles")]]
	    [[= mars::imgui::slider(1u, 128u)]] uint32_t max_triangles = 128;
};

struct meshlet_build_result {
	std::vector<meshlet> meshlets;
};

template <typename vertex_type>
meshlet_build_result build_meshlets(
    std::span<const vertex_type> vertices,
    std::span<const uint32_t> indices,
    const meshlet_build_settings& settings = {}) {
	detail::vertex_layout<vertex_type>::validate();

	const detail::raw_meshlet_build_input input = {
	    .vertex_bytes = std::as_bytes(vertices),
	    .vertex_stride = sizeof(vertex_type),
	    .position_offset = detail::vertex_layout<vertex_type>::position_offset(),
	    .indices = indices,
	    .max_vertices = settings.max_vertices,
	    .max_triangles = settings.max_triangles,
	};
	return detail::build_meshlets_raw(input);
}

} // namespace mars::mesh
