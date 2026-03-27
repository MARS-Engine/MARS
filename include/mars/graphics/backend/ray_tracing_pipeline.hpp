#pragma once

#include <mars/graphics/backend/pipeline.hpp> // reuses pipeline_descriptior_layout (preserving existing spelling)
#include <mars/graphics/backend/shader.hpp>
#include <mars/graphics/backend/ray_tracing_types.hpp>
#include <mars/meta/type_erased.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mars {
struct device;
struct command_buffer;
struct graphics_backend_functions;

struct ray_tracing_pipeline {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
};

// Minimal hit group containing only a closest-hit shader, any-hit and intersection shaders can be added in future iterations.
struct rt_hit_group {
	shader closest_hit_shader;
};

struct ray_tracing_pipeline_setup {
	shader raygen_shader;
	std::vector<shader> miss_shaders;
	std::vector<rt_hit_group> hit_groups;
	std::vector<pipeline_descriptior_layout> descriptors;
	size_t push_constant_count = 0;
	uint32_t max_recursion_depth = 1;
	uint32_t max_payload_size = 32;
	uint32_t max_attribute_size = 8;
};

struct ray_tracing_pipeline_impl {
	ray_tracing_pipeline (*ray_tracing_pipeline_create)(const device& _device, const ray_tracing_pipeline_setup& _setup) = nullptr;
	void (*ray_tracing_pipeline_bind)(const ray_tracing_pipeline& _pipeline, const command_buffer& _command_buffer) = nullptr;
	void (*ray_tracing_pipeline_destroy)(ray_tracing_pipeline& _pipeline, const device& _device) = nullptr;

	void (*rt_pipeline_write_shader_identifiers)(const ray_tracing_pipeline& _pipeline, const device& _device, uint32_t _first_group, uint32_t _count, void* _dst, uint32_t _dst_stride) = nullptr;
};
} // namespace mars
