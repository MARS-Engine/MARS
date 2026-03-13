#pragma once

#include <mars/graphics/backend/descriptor.hpp>

namespace mars::graphics::vk {
struct vk_descriptor_impl {
	static descriptor vk_descriptor_create(const device& _device, const descriptor_create_params& _params, size_t _frames_in_flight);
	static descriptor_set vk_descriptor_set_create(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params);
	static descriptor_set vk_descriptor_set_create_compute(const descriptor& _descriptor, const device& _device, const compute_pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params);
	static void vk_descriptor_set_bind(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const pipeline& _pipeline, size_t _current_frame);
	static void vk_descriptor_set_bind_compute(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const compute_pipeline& _pipeline, size_t _current_frame);
	static void vk_descriptor_set_update_cbv(descriptor_set& _descriptor_set, size_t _binding, const buffer& _buffer);
	static void vk_descriptor_destroy(descriptor& _descriptor, const device& _device);
};
} // namespace mars::graphics::vk
