#pragma once

#include <mars/graphics/backend/ray_tracing_pipeline.hpp>

namespace mars::graphics::dx {
struct dx_ray_tracing_pipeline_impl {
	static ray_tracing_pipeline dx_ray_tracing_pipeline_create(const device& _device, const ray_tracing_pipeline_setup& _setup);
	static void dx_ray_tracing_pipeline_bind(const ray_tracing_pipeline& _pipeline, const command_buffer& _command_buffer);
	static void dx_ray_tracing_pipeline_destroy(ray_tracing_pipeline& _pipeline, const device& _device);
	static void dx_rt_pipeline_write_shader_identifiers(const ray_tracing_pipeline& _pipeline, const device& _device, uint32_t _first_group, uint32_t _count, void* _dst, uint32_t _dst_stride);
};
} // namespace mars::graphics::dx
