#pragma once

#include <mars/graphics/backend/render_pass.hpp>
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

  public:
	main_pass_object(const device& dev, const object::swapchain& sc)
		: m_device(dev), m_pass_builder(dev, sc.format()), m_framebuffers(swapchain_framebuffer_set<PassTag>::create(dev, sc, m_pass_builder.get())) {}

	~main_pass_object() { destroy_all(); }

	main_pass_object(const main_pass_object&) = delete;
	main_pass_object& operator=(const main_pass_object&) = delete;

	const render_pass& get_render_pass() const { return m_pass_builder.get(); }

	void recreate_framebuffers(const device& dev, const object::swapchain& sc) {
		m_device = dev;
		m_framebuffers.recreate(dev, sc, m_pass_builder.get());
	}

	void release_framebuffers() {
		m_framebuffers.destroy_all();
	}

	const swapchain_framebuffer_handle<PassTag>& framebuffer(size_t image_index) const {
		return m_framebuffers[image_index];
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
		m_framebuffers.destroy_all();
		m_pass_builder.destroy();
		m_device = {};
	}
};

} // namespace mars::graphics::object
