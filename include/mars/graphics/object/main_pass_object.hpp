#pragma once

#include <mars/graphics/backend/render_pass.hpp>
#include <mars/graphics/object/depth_buffer.hpp>
#include <mars/graphics/object/framebuffer_typed.hpp>
#include <mars/graphics/object/render_pass_builder.hpp>
#include <mars/graphics/object/swapchain.hpp>

namespace mars::graphics::object {

template <typename PassTag>
class main_pass_object {
	static_assert(pass_desc_traits<PassTag>::uses_swapchain, "main_pass_object<PassTag>: PassTag must be annotated with [[=mars::prop::rp_uses_swapchain()]]");

	device m_device{};
	mars::render_pass_builder<PassTag> m_pass_builder;
	swapchain_framebuffer_set<PassTag> m_framebuffers;
	std::vector<object::depth_buffer> m_depth_buffers;

  public:
	main_pass_object(const device& dev, const object::swapchain& sc)
		: m_device(dev), m_pass_builder(dev, sc.format()), m_framebuffers(swapchain_framebuffer_set<PassTag>::create(dev, sc, m_pass_builder.get())) {
		recreate_depth_buffers(dev, sc);
	}

	~main_pass_object() { destroy_all(); }

	main_pass_object(const main_pass_object&) = delete;
	main_pass_object& operator=(const main_pass_object&) = delete;

	const render_pass& get_render_pass() const { return m_pass_builder.get(); }

	void recreate_framebuffers(const device& dev, const object::swapchain& sc) {
		m_device = dev;
		m_framebuffers.recreate(dev, sc, m_pass_builder.get());
		recreate_depth_buffers(dev, sc);
	}

	void release_framebuffers() {
		m_framebuffers.destroy_all();
		m_depth_buffers.clear();
	}

	const swapchain_framebuffer_handle<PassTag>& framebuffer(size_t image_index) const {
		return m_framebuffers[image_index];
	}

	const mars::depth_buffer* depth(size_t image_index) const {
		if constexpr (pass_desc_traits<PassTag>::has_depth_attachment) {
			assert(m_depth_buffers.size() == m_framebuffers.size() && "depth buffer count must match framebuffer count");
			return image_index < m_depth_buffers.size() ? &m_depth_buffers[image_index].get() : nullptr;
		}
		return nullptr;
	}

	render_pass_bind_param default_bind_params(size_t image_index) const {
		return {
			.image_index = image_index,
			.clear_color = pass_desc_traits<PassTag>::clear_color,
			.clear_depth = pass_desc_traits<PassTag>::clear_depth,
		};
	}

	bool should_present() const {
		return pass_desc_traits<PassTag>::present_enabled;
	}

	void destroy_all() {
		if (!m_device.engine)
			return;
		m_depth_buffers.clear();
		m_framebuffers.destroy_all();
		m_pass_builder.destroy();
		m_device = {};
	}

  private:
	void recreate_depth_buffers(const device& dev, const object::swapchain& sc) {
		m_depth_buffers.clear();
		if constexpr (!pass_desc_traits<PassTag>::has_depth_attachment)
			return;

		mars::depth_buffer_create_params params = {};
		params.size = sc.size();
		params.format = pass_desc_traits<PassTag>::depth_format;
		params.clear_depth = pass_desc_traits<PassTag>::clear_depth;
		// TODO: Drive this from pass traits once non-sampled depth-only passes
		// need to skip SRV allocation and post-pass shader-read transitions.
		params.sampled = true;
		m_depth_buffers.reserve(sc.swapchain_size());
		for (size_t index = 0; index < sc.swapchain_size(); ++index)
			m_depth_buffers.emplace_back(dev, params);
	}
};

} // namespace mars::graphics::object
