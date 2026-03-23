#pragma once

#include <mars/graphics/backend/render_pass.hpp>
#include <mars/graphics/functional/render_pass.hpp>
#include <mars/graphics/object/schema.hpp>
#include <mars/math/vector2.hpp>
#include <mars/meta.hpp>

#include <initializer_list>
#include <meta>
#include <utility>
#include <vector>

namespace mars {

template <typename T>
class render_pass_builder {
  private:
	render_pass m_pass;
	device m_device;
	vector2<size_t> m_fixed_size = {};

	void apply_tag_size() {
		if constexpr (meta::has_annotation<graphics::rp_size>(^^T)) {
			constexpr auto sz = meta::get_annotation<graphics::rp_size>(^^T).value();
			m_fixed_size = {sz.width, sz.height};
		}
	}

	render_pass_create_params default_params_from_tag(mars_format_type _fmt_override) {
		render_pass_create_params params;
		if constexpr (meta::has_annotation<graphics::rp_color_attachment>(^^T)) {
			constexpr auto ca = meta::get_annotation<graphics::rp_color_attachment>(^^T).value();
			params.color_formats = {(_fmt_override != MARS_FORMAT_UNDEFINED) ? _fmt_override : ca.format};
			params.load_operation = ca.load_op;
		}
		else {
			params.color_formats = {_fmt_override};
		}

		if constexpr (meta::has_annotation<graphics::rp_depth_attachment>(^^T)) {
			constexpr auto da = meta::get_annotation<graphics::rp_depth_attachment>(^^T).value();
			params.depth_format = da.format;
		}

		if constexpr (meta::has_annotation<graphics::rp_clear_depth>(^^T)) {
			constexpr auto cd = meta::get_annotation<graphics::rp_clear_depth>(^^T).value();
			params.depth_clear_value = cd.value;
		}
		return params;
	}

	void create_from_params(render_pass_create_params _params) {
		apply_tag_size();
		m_pass = graphics::render_pass_create(m_device, _params);
	}

  public:
	explicit render_pass_builder(const device& _dev, mars_format_type _fmt_override = MARS_FORMAT_UNDEFINED)
		: m_device(_dev) {
		create_from_params(default_params_from_tag(_fmt_override));
	}

	explicit render_pass_builder(const device& _dev, render_pass_create_params _params)
		: m_device(_dev) {
		create_from_params(std::move(_params));
	}

	render_pass_builder(const device& _dev, std::vector<mars_format_type> _color_formats, mars_render_pass_load_op _load_operation = MARS_RENDER_PASS_LOAD_OP_CLEAR)
		: m_device(_dev) {
		render_pass_create_params params;
		params.color_formats = std::move(_color_formats);
		params.load_operation = _load_operation;
		create_from_params(std::move(params));
	}

	render_pass_builder(const device& _dev, std::initializer_list<mars_format_type> _color_formats, mars_render_pass_load_op _load_operation = MARS_RENDER_PASS_LOAD_OP_CLEAR)
		: render_pass_builder(_dev, std::vector<mars_format_type>(_color_formats), _load_operation) {
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
