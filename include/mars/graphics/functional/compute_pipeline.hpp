#pragma once

#include <mars/graphics/backend/compute_pipeline.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars::graphics {
inline compute_pipeline compute_pipeline_create(const device& _device, const compute_pipeline_setup& _setup) {
	return _device.engine->get_impl<compute_pipeline_impl>().compute_pipeline_create(_device, _setup);
}

inline void compute_pipeline_bind(const compute_pipeline& _pipeline, const command_buffer& _command_buffer) {
	return _pipeline.engine->get_impl<compute_pipeline_impl>().compute_pipeline_bind(_pipeline, _command_buffer);
}

inline void compute_pipeline_destroy(compute_pipeline& _pipeline, const device& _device) {
	return _pipeline.engine->get_impl<compute_pipeline_impl>().compute_pipeline_destroy(_pipeline, _device);
}
} // namespace mars::graphics
