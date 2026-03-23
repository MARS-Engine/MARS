#pragma once

#include "format.hpp"
#include "shader.hpp"

#include <mars/utility/enum_flags.hpp>

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/math/vector2.hpp>
#include <mars/meta/type_erased.hpp>
#include <cstdint>
#include <vector>

enum mars_pipeline_input_advance_type {
	MARS_PIPELINE_INPUT_ADVANCE_TYPE_VERTEX,
	MARS_PIPELINE_INPUT_ADVANCE_TYPE_INSTANCE,
};

enum mars_pipeline_stage : uint32_t {
	MARS_PIPELINE_STAGE_NONE = 0u,
	MARS_PIPELINE_STAGE_VERTEX = 1u << 0,
	MARS_PIPELINE_STAGE_FRAGMENT = 1u << 1,
	MARS_PIPELINE_STAGE_COMPUTE = 1u << 2,
	MARS_PIPELINE_STAGE_VERTEX_FRAGMENT = MARS_PIPELINE_STAGE_VERTEX | MARS_PIPELINE_STAGE_FRAGMENT,
};

template <>
struct mars::enum_flags::enabled<mars_pipeline_stage> : std::true_type {};

enum mars_pipeline_descriptor_type {
	MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	MARS_PIPELINE_DESCRIPTOR_TYPE_IMAGE_SAMPLER,
	MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
	MARS_PIPELINE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
};

enum mars_compare_op {
	MARS_COMPARE_OP_NEVER,
	MARS_COMPARE_OP_LESS,
	MARS_COMPARE_OP_EQUAL,
	MARS_COMPARE_OP_LESS_EQUAL,
	MARS_COMPARE_OP_GREATER,
	MARS_COMPARE_OP_NOT_EQUAL,
	MARS_COMPARE_OP_GREATER_EQUAL,
	MARS_COMPARE_OP_ALWAYS,
};

enum mars_pipeline_primitive_topology {
	MARS_PIPELINE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	MARS_PIPELINE_PRIMITIVE_TOPOLOGY_LINE_LIST,
	MARS_PIPELINE_PRIMITIVE_TOPOLOGY_POINT_LIST,
};

namespace mars {
struct render_pass;
struct command_buffer;

struct pipeline {
	graphics_backend_functions* engine;
	meta::type_erased_ptr data;
};

struct viewport {
	mars::vector2<size_t> size;
};

struct pipeline_attribute_description {
	size_t binding;
	size_t location;
	size_t offset;
	mars_format_type input_format;
	std::string semantic_name;
	size_t semantic_index = 0;
};

struct pipeline_binding_description {
	size_t stride;
	size_t binding;
	mars_pipeline_input_advance_type type = MARS_PIPELINE_INPUT_ADVANCE_TYPE_VERTEX;
};

struct pipeline_descriptior_layout {
	mars_pipeline_stage stage;
	mars_pipeline_descriptor_type descriptor_type;
	size_t binding;
};

struct pipeline_setup {
	shader pipeline_shader;
	std::vector<pipeline_binding_description> bindings;
	std::vector<pipeline_attribute_description> attributes;
	std::vector<pipeline_descriptior_layout> descriptors;
	mars_pipeline_primitive_topology primitive_topology = MARS_PIPELINE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	size_t push_constant_count = 0;
	mars_pipeline_stage push_constant_stage = MARS_PIPELINE_STAGE_FRAGMENT;
	bool has_depth_test_override = false;
	bool depth_test_enable = true;
	bool has_depth_write_override = false;
	bool depth_write_enable = true;
	bool has_depth_compare_override = false;
	mars_compare_op depth_compare = MARS_COMPARE_OP_LESS;
	bool has_alpha_blend_override = false;
	bool alpha_blend_enable = false;
};

struct pipeline_bind_params {
	mars::vector2<size_t> size;
};

struct pipeline_impl {
	pipeline (*pipeline_create)(const device& _device, const render_pass& _render_pass, const pipeline_setup& _setup) = nullptr;
	void (*pipeline_bind)(const pipeline& _pipeline, const command_buffer& _command_buffer, const pipeline_bind_params& _params) = nullptr;
	void (*pipeline_destroy)(pipeline& _pipeline, const device& _device) = nullptr;
};
} // namespace mars
