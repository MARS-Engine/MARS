#pragma once

#include <mars/graphics/backend/ray_tracing_pipeline.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars::graphics {

inline ray_tracing_pipeline ray_tracing_pipeline_create(const device& _device, const ray_tracing_pipeline_setup& _setup) {
	return _device.engine->get_impl<ray_tracing_pipeline_impl>().ray_tracing_pipeline_create(_device, _setup);
}

inline void ray_tracing_pipeline_bind(const ray_tracing_pipeline& _pipeline, const command_buffer& _command_buffer) {
	return _pipeline.engine->get_impl<ray_tracing_pipeline_impl>().ray_tracing_pipeline_bind(_pipeline, _command_buffer);
}

inline void ray_tracing_pipeline_destroy(ray_tracing_pipeline& _pipeline, const device& _device) {
	return _pipeline.engine->get_impl<ray_tracing_pipeline_impl>().ray_tracing_pipeline_destroy(_pipeline, _device);
}

inline void rt_pipeline_write_shader_identifiers(const ray_tracing_pipeline& _pipeline, const device& _device, uint32_t _first_group, uint32_t _count, void* _dst, uint32_t _dst_stride) {
	return _pipeline.engine->get_impl<ray_tracing_pipeline_impl>().rt_pipeline_write_shader_identifiers(
		_pipeline, _device, _first_group, _count, _dst, _dst_stride);
}

} // namespace mars::graphics
