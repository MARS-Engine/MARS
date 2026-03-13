#pragma once

#include <mars/graphics/backend/compute_pipeline.hpp>

namespace mars::graphics::vk {
struct vk_compute_pipeline_impl {
	static compute_pipeline vk_compute_pipeline_create(const device& _device, const compute_pipeline_setup& _setup);
	static void vk_compute_pipeline_bind(const compute_pipeline& _pipeline, const command_buffer& _command_buffer);
	static void vk_compute_pipeline_destroy(compute_pipeline& _pipeline, const device& _device);
};
} // namespace mars::graphics::vk
