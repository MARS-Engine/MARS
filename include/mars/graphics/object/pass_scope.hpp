#pragma once

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/compute_pipeline.hpp>
#include <mars/graphics/backend/descriptor.hpp>
#include <mars/graphics/backend/framebuffer.hpp>
#include <mars/graphics/backend/pipeline.hpp>
#include <mars/graphics/backend/render_pass.hpp>
#include <mars/graphics/functional/texture.hpp>
#include <mars/graphics/functional/buffer.hpp>
#include <mars/graphics/functional/command_pool.hpp>
#include <mars/graphics/functional/descriptor.hpp>
#include <mars/graphics/functional/render_pass.hpp>
#include <mars/graphics/object/framebuffer_typed.hpp>
#include <mars/graphics/object/pipeline_factory.hpp>
#include <mars/meta.hpp>

#include <type_traits>

namespace mars {

template <typename LayoutT>
class bound_raster_set {
	mars::command_buffer* m_cmd = nullptr;

  public:
	explicit bound_raster_set(mars::command_buffer& cmd) : m_cmd(&cmd) {}

	bound_raster_set(const bound_raster_set&) = delete;
	bound_raster_set& operator=(const bound_raster_set&) = delete;

	template <typename MeshLayoutT>
	void draw_indexed(mars::mesh_data<MeshLayoutT>& mesh) {
		static_assert(std::is_same_v<LayoutT, MeshLayoutT>, "draw_indexed(mesh): mesh layout must match the currently bound descriptor set layout");

		for (auto& buf : mesh.get_buffers())
			mars::graphics::buffer_bind(buf, *m_cmd);
		mars::graphics::buffer_bind_index(mesh.get_index_buffer(), *m_cmd);
		mars::graphics::command_buffer_draw_indexed(*m_cmd, {
																.index_count = static_cast<uint32_t>(mesh.get_index_count()),
																.instance_count = 1,
																.first_index = 0,
																.vertex_offset = 0,
																.first_instance = 0,
															});
	}

	mars::command_buffer& get() { return *m_cmd; }
};

class compute_scope {
	mars::command_buffer* m_cmd = nullptr;

  public:
	explicit compute_scope(mars::command_buffer& cmd) : m_cmd(&cmd) {}

	compute_scope(const compute_scope&) = delete;
	compute_scope& operator=(const compute_scope&) = delete;

	void bind_set(const mars::descriptor_set& set, const mars::compute_pipeline& pl, size_t frame = 0) {
		mars::graphics::descriptor_set_bind_compute(set, *m_cmd, pl, frame);
	}

	void dispatch(const mars::command_buffer_dispatch_params& params) {
		mars::graphics::command_buffer_dispatch(*m_cmd, params);
	}

	template <typename T>
	void set_push_constants(const mars::compute_pipeline& pl, const T& pc) {
		mars::graphics::command_buffer_set_compute_push_constants(*m_cmd, pl, pc);
	}

	void transition(mars::texture& tex, mars_texture_state before, mars_texture_state after) {
		mars::graphics::texture_transition(*m_cmd, tex, before, after);
	}

	mars::command_buffer& get() { return *m_cmd; }
};

template <typename PassTag>
class raster_scope {
	mars::command_buffer* m_cmd = nullptr;
	const mars::render_pass* m_rp = nullptr;
	mars::vector2<size_t> m_window_size = {};
	bool m_began = false;

	void assert_expected_extent(const mars::vector2<size_t>& extent) {
		if constexpr (mars::graphics::object::pass_desc_traits<PassTag>::uses_swapchain)
			mars::logger::assert_(extent.x == m_window_size.x && extent.y == m_window_size.y, mars::log_channel("framebuffer"), "swapchain framebuffer extent mismatch: got {}x{}, expected {}x{}", extent.x, extent.y, m_window_size.x, m_window_size.y);

		if constexpr (mars::graphics::object::pass_desc_traits<PassTag>::has_fixed_size) {
			constexpr auto expected = mars::graphics::object::pass_desc_traits<PassTag>::annotated_size;
			mars::logger::assert_(extent.x == expected.x && extent.y == expected.y, mars::log_channel("framebuffer"), "fixed-size framebuffer extent mismatch: got {}x{}, expected {}x{}", extent.x, extent.y, expected.x, expected.y);
		}
	}

  public:
	raster_scope(mars::command_buffer& cmd, const mars::render_pass& rp, mars::vector2<size_t> window_size)
		: m_cmd(&cmd), m_rp(&rp), m_window_size(window_size) {}

	~raster_scope() {
		if (m_began)
			mars::graphics::render_pass_unbind(*m_rp, *m_cmd);
	}

	raster_scope(const raster_scope&) = delete;
	raster_scope& operator=(const raster_scope&) = delete;

	void begin(const mars::graphics::object::framebuffer_handle<PassTag>& fb, const mars::render_pass_bind_param& params) {
		static_assert(!mars::graphics::object::pass_desc_traits<PassTag>::uses_swapchain, "raster_scope::begin(framebuffer_handle): PassTag uses swapchain; use swapchain_framebuffer_handle instead");

		assert_expected_extent(fb.value.extent);
		mars::graphics::render_pass_bind(*m_rp, *m_cmd, fb.value, params);
		m_began = true;
	}

	void begin(const mars::graphics::object::swapchain_framebuffer_handle<PassTag>& fb, const mars::render_pass_bind_param& params) {
		static_assert(mars::graphics::object::pass_desc_traits<PassTag>::uses_swapchain, "raster_scope::begin(swapchain_framebuffer_handle): PassTag is not swapchain-backed");

		assert_expected_extent(fb.value.extent);
		mars::graphics::render_pass_bind(*m_rp, *m_cmd, fb.value, params);
		m_began = true;
	}

	template <typename LayoutT>
	bound_raster_set<LayoutT> bind_set(const mars::owned_descriptor_set<LayoutT>& set, const mars::pipeline& pl, size_t frame = 0) {
		mars::graphics::descriptor_set_bind(set.set, *m_cmd, pl, frame);
		return bound_raster_set<LayoutT>(*m_cmd);
	}

	void bind_vertex_buffer(mars::buffer& buf) {
		mars::graphics::buffer_bind(buf, *m_cmd);
	}

	void bind_index_buffer(mars::buffer& buf) {
		mars::graphics::buffer_bind_index(buf, *m_cmd);
	}

	template <typename T>
	void set_push_constants(const mars::pipeline& pl, const T& pc) {
		mars::graphics::command_buffer_set_push_constants(*m_cmd, pl, pc);
	}

	void transition(mars::texture& tex, mars_texture_state before, mars_texture_state after) {
		mars::graphics::texture_transition(*m_cmd, tex, before, after);
	}

	mars::command_buffer& get() { return *m_cmd; }
};

} // namespace mars
