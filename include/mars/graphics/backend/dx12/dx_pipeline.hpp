#pragma once

#include <mars/graphics/backend/pipeline.hpp>

namespace mars::graphics::dx {
struct dx_pipeline_impl {
	static pipeline dx_pipeline_create(const device& _device, const render_pass& _render_pass, const pipeline_setup& _setup);
	static void dx_pipeline_bind(const pipeline& _pipeline, const command_buffer& _command_buffer, const pipeline_bind_params& _params);
	static void dx_pipeline_destroy(pipeline& _pipeline, const device& _device);
};
} // namespace mars::graphics::dx
