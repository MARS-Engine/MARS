#pragma once

#include <mars/graphics/functional/command_pool.hpp>
#include <mars/graphics/functional/compute_pipeline.hpp>
#include <mars/graphics/functional/pipeline.hpp>
#include <mars/graphics/object/compute_pipeline_factory.hpp>
#include <mars/graphics/object/pipeline_factory.hpp>
#include <mars/graphics/object/shader.hpp>
#include <mars/math/vector2.hpp>

#include <optional>

namespace mars {

template <typename LayoutT, typename ShaderT, typename PassDescT>
class raster_pass_node {
  public:
	using layout_type = LayoutT;
	using tag_type = PassDescT;
	static constexpr bool is_compute = false;

	raster_pass_node(const device& dev, const render_pass& rp)
		: m_device(dev),
		  m_shader(dev),
		  m_factory(dev, static_cast<const mars::shader&>(m_shader), rp),
		  m_render_pass(rp) {}

	~raster_pass_node() { destroy(); }

	raster_pass_node(const raster_pass_node&) = delete;
	raster_pass_node& operator=(const raster_pass_node&) = delete;

	void create_descriptors(size_t frames_in_flight, size_t pool_size) {
		m_desc_factory.emplace(m_factory.create_descriptor(frames_in_flight, pool_size));
	}

	const pipeline& get_pipeline() const { return m_factory.get_pipeline(); }
	const render_pass& get_render_pass() const { return m_render_pass; }
	descriptor_factory<LayoutT>& get_descriptor_factory() { return *m_desc_factory; }

	mesh_data<LayoutT> create_mesh(size_t verts, size_t idx = 0) {
		return m_factory.create_input_buffers(verts, idx);
	}

	void bind(const command_buffer& cmd, vector2<size_t> size) {
		pipeline pl = m_factory.get_pipeline();
		graphics::pipeline_bind(pl, cmd, {.size = size});
	}

	void destroy() {
		if (!m_device.engine)
			return;
		if (m_desc_factory) {
			m_desc_factory->destroy();
			m_desc_factory.reset();
		}
		m_factory.destroy();
		m_device = {};
	}

  private:
	device m_device;
	mars::graphics::object::shader<ShaderT> m_shader;
	pipeline_factory<LayoutT> m_factory;
	render_pass m_render_pass;
	std::optional<descriptor_factory<LayoutT>> m_desc_factory;
};

template <typename LayoutT, typename ShaderT>
class compute_pass_node {
  public:
	using layout_type = LayoutT;
	using tag_type = LayoutT;
	static constexpr bool is_compute = true;

	explicit compute_pass_node(const device& dev)
		: m_device(dev),
		  m_shader(dev),
		  m_factory(dev, static_cast<const mars::shader&>(m_shader)) {}

	~compute_pass_node() { destroy(); }

	compute_pass_node(const compute_pass_node&) = delete;
	compute_pass_node& operator=(const compute_pass_node&) = delete;

	void create_descriptors(size_t frames_in_flight, size_t pool_size) {
		m_desc_factory.emplace(m_factory.create_descriptor(frames_in_flight, pool_size));
	}

	const compute_pipeline& get_pipeline() const { return m_factory.get_pipeline(); }
	descriptor_factory<LayoutT>& get_descriptor_factory() { return *m_desc_factory; }

	void bind(const command_buffer& cmd) {
		graphics::compute_pipeline_bind(m_factory.get_pipeline(), cmd);
	}

	void destroy() {
		if (!m_device.engine)
			return;
		if (m_desc_factory) {
			m_desc_factory->destroy();
			m_desc_factory.reset();
		}
		m_factory.destroy();
		m_device = {};
	}

  private:
	device m_device;
	mars::graphics::object::shader<ShaderT> m_shader;
	compute_pipeline_factory<LayoutT> m_factory;
	std::optional<descriptor_factory<LayoutT>> m_desc_factory;
};

} // namespace mars
