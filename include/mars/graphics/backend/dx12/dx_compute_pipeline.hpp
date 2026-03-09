#pragma once

#include <mars/graphics/backend/compute_pipeline.hpp>

namespace mars::graphics::dx {
struct dx_compute_pipeline_impl {
	static compute_pipeline dx_compute_pipeline_create(const device& _device, const compute_pipeline_setup& _setup);
	static void dx_compute_pipeline_bind(const compute_pipeline& _pipeline, const command_buffer& _command_buffer);
	static void dx_compute_pipeline_destroy(compute_pipeline& _pipeline, const device& _device);
};
} // namespace mars::graphics::dx
