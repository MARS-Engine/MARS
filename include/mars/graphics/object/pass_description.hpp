#pragma once

#include <mars/debug/logger.hpp>
#include <mars/graphics/object/schema.hpp>
#include <mars/meta.hpp>

namespace mars::graphics::object {

template <typename PassTag>
struct pass_desc_traits {
	static constexpr bool uses_swapchain = meta::has_annotation<mars::graphics::rp_uses_swapchain>(^^PassTag);

	static constexpr bool has_size_annotation = meta::has_annotation<mars::graphics::rp_size>(^^PassTag);

	static constexpr vector2<size_t> annotated_size = []() consteval {
		if constexpr (has_size_annotation) {
			constexpr auto sz = meta::get_annotation<mars::graphics::rp_size>(^^PassTag).value();
			return vector2<size_t>{sz.width, sz.height};
		} else
			return vector2<size_t>{0, 0};
	}();

	static constexpr bool has_fixed_size = has_size_annotation && (annotated_size.x != 0 && annotated_size.y != 0);

	static constexpr bool has_clear_color_annotation = meta::has_annotation<mars::graphics::rp_clear_color>(^^PassTag);

	static constexpr vector4<float> clear_color = []() consteval {
		if constexpr (has_clear_color_annotation)
			return meta::get_annotation<mars::graphics::rp_clear_color>(^^PassTag).value().color;
		else
			return vector4<float>{0.0f, 0.0f, 0.0f, 1.0f};
	}();

	static constexpr bool has_clear_depth_annotation = meta::has_annotation<mars::graphics::rp_clear_depth>(^^PassTag);

	static constexpr bool has_depth_attachment = meta::has_annotation<mars::graphics::rp_depth_attachment>(^^PassTag);

	static constexpr mars_depth_format depth_format = []() consteval {
		if constexpr (has_depth_attachment)
			return meta::get_annotation<mars::graphics::rp_depth_attachment>(^^PassTag).value().format;
		else
			return MARS_DEPTH_FORMAT_UNDEFINED;
	}();

	static constexpr float clear_depth = []() consteval {
		if constexpr (has_clear_depth_annotation)
			return meta::get_annotation<mars::graphics::rp_clear_depth>(^^PassTag).value().value;
		else
			return 1.0f;
	}();

	static constexpr bool has_present_annotation = meta::has_annotation<mars::graphics::rp_present>(^^PassTag);

	static constexpr bool present_enabled = []() consteval {
		if constexpr (has_present_annotation)
			return meta::get_annotation<mars::graphics::rp_present>(^^PassTag).value().enabled;
		return uses_swapchain;
	}();
};

} // namespace mars::graphics::object
