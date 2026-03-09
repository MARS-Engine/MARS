#pragma once

#include <mars/graphics/backend/render_pass.hpp>
#include <mars/graphics/functional/render_pass.hpp>
#include <mars/graphics/object/schema.hpp>
#include <mars/math/vector2.hpp>
#include <mars/meta.hpp>

#include <meta>

namespace mars {

template <typename T>
class render_pass_builder {
      private:
	render_pass m_pass;
	device m_device;
	vector2<size_t> m_fixed_size = {};

      public:
	explicit render_pass_builder(const device& dev,
				     mars_format_type fmt_override = MARS_FORMAT_UNDEFINED)
	    : m_device(dev) {
		render_pass_create_params params;

		if constexpr (meta::has_annotation<graphics::rp_color_attachment>(^^T)) {
			constexpr auto ca = meta::get_annotation<graphics::rp_color_attachment>(^^T).value();
			params.format = (fmt_override != MARS_FORMAT_UNDEFINED) ? fmt_override : ca.format;
			params.load_operation = ca.load_op;
		} else {

			params.format = fmt_override;
		}

		if constexpr (meta::has_annotation<graphics::rp_depth_attachment>(^^T)) {
			constexpr auto da = meta::get_annotation<graphics::rp_depth_attachment>(^^T).value();
			params.depth_format = da.format;
		}

		if constexpr (meta::has_annotation<graphics::rp_clear_depth>(^^T)) {
			constexpr auto cd = meta::get_annotation<graphics::rp_clear_depth>(^^T).value();
			params.depth_clear_value = cd.value;
		}

		if constexpr (meta::has_annotation<graphics::rp_size>(^^T)) {
			constexpr auto sz = meta::get_annotation<graphics::rp_size>(^^T).value();
			m_fixed_size = {sz.width, sz.height};
		}

		m_pass = graphics::render_pass_create(dev, params);
	}

	~render_pass_builder() { destroy(); }

	render_pass_builder(const render_pass_builder&) = delete;
	render_pass_builder& operator=(const render_pass_builder&) = delete;

	const render_pass& get() const { return m_pass; }
	vector2<size_t> get_fixed_size() const { return m_fixed_size; }

	void destroy() {
		if (!m_pass.engine)
			return;
		mars::graphics::render_pass_destroy(m_pass, m_device);
		m_pass = {};
		m_device = {};
	}
};

} // namespace mars
