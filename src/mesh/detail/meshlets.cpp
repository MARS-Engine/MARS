#include <mars/mesh/meshlets.hpp>
#include <mars/mesh/mesh.hpp>

#include <vector>

namespace mars::mesh {

meshlet_build_result detail::build_meshlets_raw(const raw_meshlet_build_input& input) {
	meshlet_build_result result;
	if (input.vertex_stride == 0 || input.vertex_bytes.empty() || input.indices.empty() || input.indices.size() % 3 != 0)
		return result;

	const size_t vertex_count = input.vertex_bytes.size() / input.vertex_stride;
	if (vertex_count == 0)
		return result;

	const size_t max_meshlets = mesh_build_meshlets_bound(
	    input.indices.size(),
	    input.max_vertices,
	    input.max_triangles);
	std::vector<mesh_meshlet> raw_meshlets(max_meshlets);
	std::vector<unsigned int> meshlet_vertices(input.indices.size());
	std::vector<unsigned char> meshlet_triangles(input.indices.size());

	const float* positions = reinterpret_cast<const float*>(input.vertex_bytes.data() + input.position_offset);
	const size_t meshlet_count = mesh_build_meshlets(
	    raw_meshlets.data(),
	    meshlet_vertices.data(),
	    meshlet_triangles.data(),
	    input.indices.data(),
	    input.indices.size(),
	    positions,
	    vertex_count,
	    input.vertex_stride,
	    input.max_vertices,
	    input.max_triangles,
	    0.0f);

	result.meshlets.reserve(meshlet_count);
	for (size_t i = 0; i < meshlet_count; ++i) {
		const mesh_meshlet& source_meshlet = raw_meshlets[i];
		meshlet output_meshlet;
		output_meshlet.unique_vertex_indices.assign(
		    meshlet_vertices.begin() + source_meshlet.vertex_offset,
		    meshlet_vertices.begin() + source_meshlet.vertex_offset + source_meshlet.vertex_count);
		output_meshlet.local_indices.assign(
		    meshlet_triangles.begin() + source_meshlet.triangle_offset,
		    meshlet_triangles.begin() + source_meshlet.triangle_offset + source_meshlet.triangle_count * 3);
		result.meshlets.push_back(std::move(output_meshlet));
	}

	return result;
}

} // namespace mars::mesh
