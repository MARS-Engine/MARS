#pragma once

#include <mars/graphics/backend/graphics_backend.hpp>
#include <mars/graphics/backend/pipeline.hpp>

namespace mars::graphics::object {

inline mars::pipeline pipeline_create(
    const mars::device& device,
    const mars::render_pass& render_pass,
    const mars::pipeline_setup& setup) {
	return device.engine->get_impl<pipeline_impl>().pipeline_create(device, render_pass, setup);
}

inline void pipeline_destroy(mars::pipeline& pipeline, const mars::device& device) {
	pipeline.engine->get_impl<pipeline_impl>().pipeline_destroy(pipeline, device);
}

} // namespace mars::graphics::object
