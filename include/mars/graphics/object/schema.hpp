#pragma once

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/compute_pipeline.hpp>
#include <mars/graphics/backend/format.hpp>
#include <mars/graphics/backend/pipeline.hpp>
#include <mars/math/vector4.hpp>

#include <cstddef>
#include <cstdint>

namespace mars {
namespace graphics {
struct input {
	size_t binding;
	mars_pipeline_stage stage;
};

struct uniform {
	size_t binding;
	mars_pipeline_stage stage;
};

struct push_constants {
	size_t count;
	mars_pipeline_stage stage;
};

struct push_constants_type {
	size_t byte_count;
	mars_pipeline_stage stage;
};

struct writes_uav {
};

template <typename ProducerTag>
struct reads_from {
	mars_texture_state required_state = MARS_TEXTURE_STATE_SHADER_READ;
};

struct rp_color_attachment {
	mars_format_type format;
	mars_render_pass_load_op load_op = MARS_RENDER_PASS_LOAD_OP_CLEAR;
};

struct rp_depth_attachment {
	mars_format_type format;
};

struct rp_size {
	size_t width = 0;
	size_t height = 0;
};

struct rp_uses_swapchain {
};

struct rp_clear_color {
	vector4<float> color = {0.0f, 0.0f, 0.0f, 1.0f};
};

struct rp_clear_depth {
	float value = 1.0f;
};

struct rp_present {
	bool enabled = true;
};

struct depth_test {
	bool enabled = true;
};

struct depth_write {
	bool enabled = true;
};

struct depth_compare {
	mars_compare_op op = MARS_COMPARE_OP_LESS;
};

struct blend_alpha {
	bool enabled = true;
};

struct compute_input {
	size_t binding;
};

struct compute_setting {
	size_t binding;
};

struct compute_output {
	size_t binding;
};
} // namespace graphics

namespace prop {
static constexpr graphics::input input(size_t binding, mars_pipeline_stage stage = MARS_PIPELINE_STAGE_VERTEX) { return {.binding = binding, .stage = stage}; }
static constexpr graphics::uniform uniform(size_t binding, mars_pipeline_stage stage = MARS_PIPELINE_STAGE_VERTEX) { return {.binding = binding, .stage = stage}; }
static constexpr graphics::push_constants push_constants(size_t count, mars_pipeline_stage stage = MARS_PIPELINE_STAGE_FRAGMENT) { return {.count = count, .stage = stage}; }

template <typename T>
static consteval graphics::push_constants_type push_constants_type(mars_pipeline_stage stage = MARS_PIPELINE_STAGE_FRAGMENT) {
	return {.byte_count = sizeof(T) / sizeof(uint32_t), .stage = stage};
}

static consteval graphics::writes_uav writes_uav() { return {}; }

template <typename ProducerTag>
static consteval graphics::reads_from<ProducerTag> reads_from(mars_texture_state required = MARS_TEXTURE_STATE_SHADER_READ) {
	return {.required_state = required};
}

static consteval graphics::rp_color_attachment rp_color_attachment(mars_format_type fmt, mars_render_pass_load_op op = MARS_RENDER_PASS_LOAD_OP_CLEAR) {
	return {.format = fmt, .load_op = op};
}
static consteval graphics::rp_depth_attachment rp_depth_attachment(mars_format_type fmt) {
	return {.format = fmt};
}
static consteval graphics::rp_size rp_size(size_t width = 0, size_t height = 0) {
	return {.width = width, .height = height};
}
static consteval graphics::rp_uses_swapchain rp_uses_swapchain() { return {}; }
static consteval graphics::rp_clear_color rp_clear_color(float r, float g, float b, float a = 1.0f) {
	return {.color = {r, g, b, a}};
}
static consteval graphics::rp_clear_depth rp_clear_depth(float value = 1.0f) {
	return {.value = value};
}
static consteval graphics::rp_present rp_present(bool enabled = true) {
	return {.enabled = enabled};
}
static consteval graphics::depth_test depth_test(bool enabled = true) {
	return {.enabled = enabled};
}
static consteval graphics::depth_write depth_write(bool enabled = true) {
	return {.enabled = enabled};
}
static consteval graphics::depth_compare depth_compare(mars_compare_op op = MARS_COMPARE_OP_LESS) {
	return {.op = op};
}
static consteval graphics::blend_alpha blend_alpha(bool enabled = true) {
	return {.enabled = enabled};
}
static constexpr graphics::compute_input compute_input(size_t binding) { return {.binding = binding}; }
static constexpr graphics::compute_setting compute_setting(size_t binding) { return {.binding = binding}; }
static constexpr graphics::compute_output compute_output(size_t binding) { return {.binding = binding}; }
} // namespace prop
} // namespace mars
